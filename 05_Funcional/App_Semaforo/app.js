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
    firmware: {}, // NODE -> sello FW de ESTE enlace (js/exportar.js); se olvida con el
    modo: null, // 'AUTO', 'MANUAL', 'AMBAR', 'ROJO_TOTAL'
    estadoLuces: null, // V1_R2, Y1_R2, R1_R2, R1_V2, R1_Y2, AMBAR_FAIL, ALL_RED
    // LO QUE EL MAESTRO SABE DEL ESCLAVO (N-149, 05/09). Viaja en el campo ESC: del
    // $STATUS del MAESTRO y SOLO de el; el Esclavo no lo emite -no tiene a quien
    // preguntarle-. Tres valores distintos y ninguno se colapsa con otro:
    //
    //   null   el campo NO VINO. Es un equipo con firmware anterior a N-149, o el
    //          $STATUS de un Esclavo. La app no puede saber nada del otro poste.
    //   '?'    vino, y el MAESTRO declara que NO LO SABE: enlace de radio caido, o
    //          todavia sin la primera respuesta del Esclavo.
    //   ROJO / VERDE / AMBAR   lo que el Maestro tiene por ultimo dato del otro poste.
    //
    // Los dos primeros se pintan IGUAL de vacios y se DICEN distinto, porque mandan a
    // sitios distintos: uno es "actualice el firmware", el otro es "vaya a mirar la
    // radio". Lo que no se hace en ninguno de los dos es dejar el semaforo del POSTE 2
    // con el ultimo color pintado como si fuera de ahora (CLAUDE.md 3.quinquies).
    esc: null,
    // LA TALANQUERA DEL POSTE CON EL QUE SE HABLA (N-153, 05/09). Viaja en el campo
    // PLUMA: del $STATUS de LAS DOS puntas -las dos placas son la misma y las dos
    // tienen su motor en PB2-, asi que este dato es SIEMPRE el del poste que habla y
    // nunca el del otro. Cuatro valores y ninguno se colapsa con otro:
    //
    //   null       el campo NO VINO: equipo con firmware anterior a N-153.
    //   'ARRIBA'   la barrera esta levantada: por debajo se pasa.
    //   'ABAJO'    la barrera esta bajada.
    //   otra cosa  un literal que esta app no conoce. Se ensena EN CRUDO, no se adivina.
    //
    // POR QUE IMPORTA QUE SE VEA: es el unico elemento del equipo que SE MUEVE, y el
    // conductor le hace mas caso que a la lampara -por eso se pone-. Hasta hoy la app lo
    // nombraba solo dentro de los textos que explican botones, nunca como estado, asi
    // que el operario no podia saber si estaba arriba o abajo AHORA.
    pluma: null,
    // LO QUE EL EQUIPO SABE DE SUS CAMARAS (D-13 fase 1, 05/09). Viaja en el campo
    // CAM: del $STATUS de LAS DOS puntas y es LA PEOR de las dos camaras de ese
    // poste. Cinco salidas y ninguna se colapsa con otra:
    //
    //   null       el campo NO VINO: equipo con firmware anterior a D-13.
    //   'OK'       las dos ven y ninguna esta pegada.
    //   '?'        el EQUIPO dice que todavia no consta que vean. NO es 'OK' y
    //              tampoco es null: null es que la app no tiene dato.
    //   'CIEGA'    lleva demasiado paso abierto sin una sola deteccion. El umbral NO
    //              se nombra aqui: vive en CAM_CIEGA_MS del C++ y ya cambio una vez
    //              -6 h -> 24 h el 08/09-. Una cifra copiada a este lado no se entera.
    //   'PEGADA'   contacto fijo: afirma presencia y no se suelta.
    //
    // POR QUE IMPORTA QUE SE VEA: en fase 1 el vigilante no toca el ciclo, solo
    // cuenta. O sea que una camara pegada NO se nota en la calle -el cruce sigue
    // ciclando- y la unica forma de enterarse es esta. Y en fase 2, de este dato
    // cuelga si la pluma baja.
    cam: null,
    // null = NADIE HA DICHO CUANTO FALTA. No es 0: 0 es el ultimo segundo de la
    // fase, que es un dato, y esto es la ausencia de dato. Ver N-139 y
    // updateCountdownRing().
    // El ultimo equipo al que se abrio enlace, para poder VOLVER a el sin buscar. No es
    // lo mismo que deviceName/deviceMac, que dicen "el elegido ahora": esto sobrevive a
    // la caida, que es justo cuando hace falta.
    ultimoEquipo: null,
    countdown: null,
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
    // LA HORA QUE EL EQUIPO DICE TENER. Hasta el 05/09 esta variable tenia UN escritor
    // y CERO lectores: el campo HORA: llegaba en cada $STATUS, se guardaba aqui y no lo
    // pintaba nadie. Es la version de dato de la funcion huerfana de CLAUDE.md 3.bis, y
    // lo destapo un censo -no una queja de campo-, que es como hay que encontrarlas.
    // El lector es pintarHoraEquipo(). Tres valores y ninguno se colapsa con otro:
    // null = el equipo no ha hablado; "--:--:--" = el equipo dice que NO tiene la hora
    // puesta (y de esa hora cuelga la autorizacion del Modo Degradado); HH:MM:SS = la
    // hora de su DS3231.
    hora: null,
    pin: '',
    correctPin: '1234',
    // N-75: la app conocia el PIN y lo inyectaba en TODOS los comandos, asi que la
    // barrera del firmware -que guarda lo que ABRE paso o mueve luces- la abria ella
    // misma. Un comando que sale con un PIN que el operario no tecleo no es una
    // autorizacion. Esta bandera dice si alguien lo puso en esta sesion.
    pinVerificado: false,
    accionPendiente: null,
    // LA AUTORIZACION CADUCA (04/09). Hasta hoy `pinVerificado` se ponia a `true` en
    // UNA linea y no se apagaba en NINGUNA: ni al cerrar el teclado, ni al cambiar de
    // punta, ni al caerse el enlace, ni con el tiempo. El operario teclea, se guarda
    // el telefono, y el siguiente que lo coja manda ordenes sin teclear nada. Lo dejo
    // escrito el propio arnes de DOM -"hoy dura lo que dure el proceso"- y quien
    // decide cuanto tiene que durar es el responsable, que lo decidio el 04/09.
    //
    //   ocultaDesdeMs   instante en que la app se fue a segundo plano, o null si esta
    //                   delante. Los 60 s de gracia se cuentan desde aqui.
    //   ultimaOrdenMs   instante de la ultima orden que SALIO al cable. No es "la
    //                   ultima vez que alguien toco la pantalla": mirar el tablero no
    //                   renueva una autorizacion para mandar.
    ocultaDesdeMs: null,
    ultimaOrdenMs: null,
    // EL VALE DE VIA DESPEJADA, que NO es el PIN y no lo sustituye en ningun otro
    // sitio. { orden, ms, fase }: que maniobra se confirmo, cuando, y en que fase
    // estaba el cruce cuando el operario miro. Las tres hacen falta para saber si lo
    // que vio sigue siendo lo que hay. Ver 4.bis.
    confirmacionVia: null,
    accionViaPendiente: null,
    ordenViaPendiente: null,
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
  const btnBtDisconnect = document.getElementById('btn-bt-disconnect');
  const panelDiagnosticoEl = document.getElementById('panel-diagnostico');
  const diagLogEl = document.getElementById('diag-log');
  // D-23: el recuadro de los contadores de radio de ESTE poste, dentro de la ventana de
  // diagnostico que ya existia. No es una pantalla nueva -la del POSTE 2 lleva abierta
  // desde N-149-: es el dato que le faltaba dentro.
  const diagRfVacioEl = document.getElementById('diag-rf-vacio');
  const diagRfDatosEl = document.getElementById('diag-rf-datos');
  const diagRfRxEl = document.getElementById('diag-rf-rx');
  const diagRfOkEl = document.getElementById('diag-rf-ok');
  const diagRfRuidoEl = document.getElementById('diag-rf-ruido');
  const diagRfRxDEl = document.getElementById('diag-rf-rx-d');
  const diagRfOkDEl = document.getElementById('diag-rf-ok-d');
  const diagRfRuidoDEl = document.getElementById('diag-rf-ruido-d');
  const diagRfPieEl = document.getElementById('diag-rf-pie');
  // El cartel de la barrera retenida por la camara (14/09). Vive FUERA de las pestanas:
  // ver el comentario de index.html y el de js/aviso_camara_pluma.js.
  const camPlumaEl = document.getElementById('aviso-camara-pluma');
  const camPlumaTituloEl = document.getElementById('camara-pluma-titulo');
  const camPlumaMedidaEl = document.getElementById('camara-pluma-medida');
  const camPlumaAccionEl = document.getElementById('camara-pluma-accion');
  const camPlumaLimiteEl = document.getElementById('camara-pluma-limite');
  const cortEl = document.getElementById('aviso-corte');   // N-170, js/aviso_corte.js
  const padTituloEl = document.getElementById('pad-titulo');
  const btnIrAlMaestro = document.getElementById('btn-ir-al-maestro');
  const btnIrAlEsclavo = document.getElementById('btn-ir-al-esclavo');
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
  const plumaEl = document.getElementById('pluma-estado');   // N-153
  const badgeModoEl = document.getElementById('badge-modo');

  // Metrics
  const rfQualityEl = document.getElementById('rf-quality');
  const rfRttEl = document.getElementById('rf-rtt');
  const rfEstadoEl = document.getElementById('rf-estado');
  const rfBarraEl = document.getElementById('rf-barra');
  const rfSelloEl = document.getElementById('rf-sello');
  const batVoltageEl = document.getElementById('bat-voltage');
  const batStatusEl = document.getElementById('bat-status');
  const camEstadoEl = document.getElementById('cam-estado');     // D-13
  const camDetalleEl = document.getElementById('cam-detalle');   // D-13

  // La caja de la Prueba de Alcance (28/09): el funcional en campo pulsaba el boton y
  // no sabia "donde se ve este valor" -el % ya se pintaba en rf-quality, pero en la
  // pestana de Operacion, mientras el boton vive en la de Tecnico-. Ver
  // pintarCajaAlcance() mas abajo.
  const alcanceCajaEl = document.getElementById('alcance-caja');
  const alcanceCajaPctEl = document.getElementById('alcance-caja-pct');
  const alcanceCajaRttEl = document.getElementById('alcance-caja-rtt');
  const alcanceCajaFraseEl = document.getElementById('alcance-caja-frase');

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
  // propia: ALCANCE viaja sin PIN y DEMANDA solo vale en un modo concreto.
  const btnModoAlcance = document.getElementById('btn-modo-alcance');
  const btnDemanda = document.getElementById('btn-demanda');
  const demandaHintEl = document.getElementById('demanda-hint');

  // El dialogo de VIA DESPEJADA. Es el que sustituye al teclado de PIN en las dos
  // ordenes que ABREN paso desde la botonera de campo. Ver 4.bis.
  const viaModal = document.getElementById('via-modal');
  const modalViaClose = document.getElementById('modal-via-close');
  const viaManiobraEl = document.getElementById('via-maniobra');
  const btnViaCancelar = document.getElementById('btn-via-cancelar');
  const btnViaConfirmar = document.getElementById('btn-via-confirmar');

  // Courier RTC Elements
  const btnCourierCapture = document.getElementById('btn-courier-capture');
  const btnCourierInject = document.getElementById('btn-courier-inject');
  const courierSnapshotText = document.getElementById('courier-snapshot-text');
  const courierTimerDigits = document.getElementById('courier-timer-digits');
  const btnSyncRtc = document.getElementById('btn-sync-rtc');
  // A-9: la consulta de SOLO LECTURA. Ver su manejador, mas abajo.
  const btnLeerRtc = document.getElementById('btn-leer-rtc');
  const btnLeerVersion = document.getElementById('btn-leer-version');
  const rtcSyncDigits = document.getElementById('rtc-sync-digits');
  // N-150: el primer LECTOR que ha tenido nunca state.hora. Ver pintarHoraEquipo().
  const equipoHoraEl = document.getElementById('equipo-hora');
  const btnStartTestLeds = document.getElementById('btn-start-test-leds');

  // Form Tiempos
  const formTiempos = document.getElementById('form-tiempos');
  const numTiempoVerde = document.getElementById('num-tiempo-verde');
  const numTiempoRojo = document.getElementById('num-tiempo-rojo');
  const numTiempoDespeje = document.getElementById('num-tiempo-despeje');
  // N-150: el cartel de "aceptado, pero el cruce sigue parado" y su mando. Ver 7.bis.
  const avisoTiemposParado = document.getElementById('aviso-tiempos-parado');
  const btnArrancarCiclo = document.getElementById('btn-arrancar-ciclo');

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
  // Las ordenes que el firmware acepta SIN autenticar, con el nombre EXACTO del cable (censo de app_01_comandos).
  // El criterio es el del despachador, no el de la app: el PIN guarda lo que ABRE paso, no lo que lo para.
  //   FORZAR_ROJO       Maestro. Rojo fijo (en Degradado, SOLO en ese poste: D-51).
  //   AMBAR_EMERGENCIA  Esclavo. Ambar intermitente y talanquera ABIERTA; el Esclavo rechaza el literal FORZAR_ROJO
  //                     nombrando este (hasta el 28/08 acusaba "rojo forzado" sin poner un rojo).
  //   SET_MODO:MENU / SET_MODO:ALCANCE   Maestro. Menu sin ciclo / rojo fijo; MENU es la vuelta atras universal.
  // Se aceptan TAMBIEN con PIN: aqui no se abre ninguna puerta, solo se pueden usar sin teclear delante de un cruce
  // parado. Vale para las DOS de emergencia: una caida segura que pide clave no es segura.
  // LEER_RTC, VERSION y las consultas (CONSULTA_DEG_*, CONSULTA_NOCHE D-110, REINICIOS D-64, LOG D-75) no abren ni
  // paran ni cambian nada: hay que poder MIRAR antes de tocar; VERSION es la primera pregunta si algo va raro.
  const SIN_PIN = ['FORZAR_ROJO', 'AMBAR_EMERGENCIA', 'SET_MODO:MENU', 'SET_MODO:ALCANCE',
                   'LEER_RTC', 'VERSION', 'CONSULTA_DEG_AUTO', 'CONSULTA_DEG_FIN', 'CONSULTA_NOCHE', 'REINICIOS',
                   'LOG,DESDE'];

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
    const orden = _ordenDeCable(comando, args);
    // La excepcion es el rojo de emergencia: bluetooth.cpp:70-82 lo acepta SIN PIN a
    // proposito -el PIN guarda lo que abre paso, no lo que lo para-.
    //
    // Y LA SEGUNDA PUERTA, del 04/09: el VALE DE VIA DESPEJADA. El firmware no cambia
    // -sigue exigiendo CMD:PIN:1234: y la app se lo sigue poniendo-, lo que cambia es
    // A QUIEN se lo pide la app. Para las dos ordenes que abren paso desde la botonera
    // de campo el operario ya no teclea una clave: contesta una pregunta sobre la via.
    // El motivo esta en 4.bis y no es de comodidad: un PIN demuestra QUIEN eres y no
    // demuestra que hayas MIRADO, que es lo unico que el equipo no puede saber.
    // 29/09: en Degradado, cambiar el modo de UN poste pide paleteros (js/aviso_degradado.js).
    if (!AvisoDegradado.permite(orden, state, addEvent)) return false;
    if (!SIN_PIN.includes(comando) && !state.pinVerificado &&
        !viaConfirmadaVigente(orden)) {
      // EL AVISO TIENE QUE NOMBRAR LA BARRERA QUE FALTO, Y HASTA HOY NOMBRABA LA OTRA.
      //
      // Este texto decia "falta autorizacion con PIN" para las dos puertas. Desde que
      // SET_MODO:AUTO y MANUAL:CAMBIAR_TURNO pasan por el vale de via, a esas dos NO
      // les falta una clave: les falta que alguien mire la calzada. Un aviso que nombra
      // la barrera equivocada manda al operario a teclear cuatro digitos que nadie le
      // va a pedir, delante de un cruce parado.
      //
      // Cual toca lo decide VIA_MANIOBRA, que es la misma tabla con la que confirmarVia()
      // redacta la pregunta: si una orden esta ahi, su puerta es la via.
      const porVia = Object.prototype.hasOwnProperty.call(VIA_MANIOBRA, orden);
      const motivo = porVia
        ? 'falta confirmar que la via esta despejada'
        : 'falta la autorizacion con PIN';
      console.warn('[TX BLOQUEADO] ' + motivo + ':', comando);
      addEvent('red', 'Comando ' + comando + ' no enviado: ' + motivo + '.' + (porVia
        ? ' Si acaba de confirmarla, el vale ya no vale: caduca a los ' +
          Math.round(VIA_VIGENCIA_MS / 1000) + ' s y tambien en cuanto el cruce cambia ' +
          'de fase. Vuelva a pulsar y conteste otra vez.'
        : ''));
      // UNA ORDEN QUE NO SALIO TAMBIEN VA AL DIARIO -y NO a la cinta-. Para el que esta
      // de pie, "pulse y no paso nada" tiene el mismo aspecto tanto si la app se planto
      // como si el equipo no contesto, y son dos averias distintas. La cinta es el
      // cable: por el cable no paso ni un byte, asi que ahi no se anota.
      DiarioOrdenes.anotarOrden(orden, null, Date.now(), { salio: false, motivo: motivo });
      renderDiario();
      // D-56: con la clave de administracion se ven Tiempos y la hora sin PIN; el teclado se abre aqui, sin orden en
      // cola (la orden no salio y no se repite sola): tecleado el PIN, se vuelve a pulsar.
      if (!porVia) { pedirPin(null); showToast('Teclee el PIN del equipo y vuelva a pulsar'); }
      return false;
    }
    const pin = state.correctPin;
    let rawCmd = '';
    if (SIN_PIN.includes(comando)) {
      rawCmd = 'CMD:' + orden + '\r\n';   // orden = comando[:args]: solo LOG,DESDE (D-75) lleva args sin PIN
    } else if (args) {
      rawCmd = `CMD:PIN:${pin}:${comando}:${args}\r\n`;
    } else {
      rawCmd = `CMD:PIN:${pin}:${comando}\r\n`;
    }

    // EL PIN SE TAPA EN LA CONSOLA TAMBIEN. No es donde mas duele -la consola no se
    // exporta-, pero un volcado de logcat viaja igual de facil que un fichero y la
    // regla es la misma: el unico sitio donde la clave va entera es el cable.
    console.log('[TX BLUETOOTH STM32]:', RegistroCrudo.taparPin(rawCmd.trim()));

    // 1. APK con Bluetooth fisico. Sin enlace abierto no hay write(): NO salio (SPEC_4, apartado 4).
    const escrita = typeof window !== 'undefined' && !!window.bluetoothSerial && state.connected;
    if (escrita) {
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
    // Lo no escrito va al Diario como "no salio" y NO a la cinta, como la frenada por PIN,
    // y devuelve false. El puente Python de arriba no cuenta: tira la orden.
    if (!escrita) {
      addEvent('red', 'Comando ' + comando + ' no enviado: no hay enlace Bluetooth abierto ' +
                      'con el equipo. Conectese al poste y repitalo.');
      DiarioOrdenes.anotarOrden(orden, null, Date.now(),
                                { salio: false, motivo: 'sin enlace Bluetooth abierto' });
      renderDiario();
      return false;
    }

    // El sello de actividad se pone AQUI y no en el manejador del boton: lo que
    // renueva la autorizacion es haber mandado algo, no haber tocado la pantalla. Un
    // telefono encendido encima del capo no manda nada, y es justo el caso que la
    // caducidad por inactividad cubre.
    const salidaMs = Date.now();
    state.ultimaOrdenMs = salidaMs;

    // SIN ENLACE NO SE CALLA - Y TAMPOCO SE BLOQUEA EL ENVIO (roadmap 1.50).
    //
    // El 16/09 en el Sisga se escribieron SEIS CMD:FORZAR_ROJO seguidos a un cable
    // muerto -11:44:12 a 11:44:25, sin una sola trama de vuelta entre ellos- y la app
    // solo dijo "sin respuesta", y solo al exportar el diario. El tecnico se fue del
    // poste creyendo que habia mandado el rojo.
    //
    // NO SE BLOQUEA, y es deliberado: el enlace puede volver en el byte siguiente, y
    // tragarse un FORZAR_ROJO es peor que mandarlo a ciegas. Lo que se arregla es que
    // el operario lo SEPA en el momento, no al exportar.
    //
    // EL BORDE ES TIMEOUT_ENLACE_MS, EL MISMO QUE PINTA "Sin enlace" EN LA CABECERA, y
    // se reusa a proposito: un umbral propio aqui seria una SEGUNDA respuesta a la misma
    // pregunta -"habia alguien al otro lado"- y el dia que difieran, la cabecera y el
    // diario contarian cosas distintas de la misma sesion.
    const mudoMs = state.ultimoStatusMs ? (salidaMs - state.ultimoStatusMs) : null;
    const sinEnlace = (mudoMs === null) || (mudoMs > TIMEOUT_ENLACE_MS);
    if (sinEnlace) {
      addEvent('red', 'SIN ENLACE al mandar ' + comando + ': se escribio al cable, pero ' +
        (mudoMs === null
          ? 'el equipo no ha hablado ni una vez en esta sesion'
          : 'el equipo lleva ' + Math.round(mudoMs / 1000) + ' s sin hablar') +
        '. NO se sabe si la orden llego. Repitala cuando el enlace vuelva.');
    }

    // ---- EL REGISTRO DE LO QUE SALE --------------------------------------
    // Se anota la trama TAL Y COMO SALIO, no un resumen: con el defecto inyectado en la
    // guarda de via solo el CONTENIDO de la trama delato el fallo. Va DESPUES de escribir
    // y solo si se escribio (`escrita`, arriba): ENVIADA significa "se escribio a la
    // salida", ni un milimetro mas. La historia de por que existe, en git log.
    RegistroCrudo.anotar(rawCmd, { enviada: true, tipo: 'CMD' }, salidaMs);
    // Y la MISMA orden abre su entrada en el diario, que es donde se le juntaran la
    // respuesta del equipo y lo que se vea cambiar despues. Son dos registros porque la
    // cinta se corta a 300 tramas y en una sola sesion de banco se tiraron 379: metiendo
    // los envios dentro, se llena antes y expulsa justo lo que se busca.
    DiarioOrdenes.anotarOrden(orden, rawCmd, salidaMs,
                              { sinEnlace: sinEnlace, mudoMs: mudoMs });
    renderDepuracion();
    renderDiario();

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
    // La ventana de diagnostico ensena los ultimos de esta misma lista, asi que se
    // repinta aqui y no en cada llamador: una segunda lista de sitios donde acordarse
    // de refrescar es una lista que alguien deja sin actualizar.
    renderDiagnostico();
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

    if (rfQualityEl) { rfQualityEl.textContent = medido ? lectura.pct + '%' : (lectura && lectura.crudo === '!' ? '!' : '--'); rfQualityEl.classList.toggle('cyan', !!medido); }  // D-83

    // rf-rtt lleva el numero que vino, y NADA MAS: la unidad la pone el rotulo fijo
    // del HTML. Si no vino, la casilla queda con dos guiones y la unidad se retira -un
    // "-- ms" es una medida en milisegundos que nadie hizo-. Quien declara la ausencia
    // es el rotulo del tramo y el sello de hora, no un valor de relleno.
    const hayRtt = lectura && lectura.rtt !== null && lectura.rtt !== undefined;
    if (rfRttEl) rfRttEl.textContent = hayRtt ? String(lectura.rtt) : (lectura && lectura.crudoRtt === '!' ? '!' : '--');
    const rfRttUnidadEl = document.getElementById('rf-rtt-unidad');
    if (rfRttUnidadEl) rfRttUnidadEl.style.visibility = hayRtt ? 'visible' : 'hidden';

    if (rfEstadoEl) {
      rfEstadoEl.textContent = ENLACE_ROTULO[tramo];
      rfEstadoEl.className = 'enlace-estado enlace-' + tramo.toLowerCase().replace('_', '');
    }
    if (rssiTextEl) rssiTextEl.textContent = !state.telemetriaViva ? '(sin enlace)'  // cabecera, 06/10: este
      : DegAutoEleccion.sinRadio(state) ? '(sin radio)'  // enlace y el sinRadio() del cartel; <= 16 (D-82)
      : medido ? '(radio ' + lectura.pct + ' %)' : '(enlazado)';

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
  // 1.ter.bis LA CAJA DE LA PRUEBA DE ALCANCE (28/09, funcional en campo)
  // =========================================================================
  // "La prueba de enlace no comprendo como funciona o donde se ve este valor": el
  // boton btn-modo-alcance vive en la pestana de Tecnico y el % que mide -rf-quality,
  // pintado por pintarEnlace() arriba- vive en la pestana de Operacion. El tecnico
  // pulsaba el boton y no tenia delante ningun numero que le dijera que la prueba
  // estaba corriendo.
  //
  // CAMBIO MINIMO: mientras el equipo diga MODO:ALCANCE, esta funcion pinta el MISMO
  // dato -mismo % y mismo RTT que ya calculo pintarEnlace()/lecturaDeEnlace()- en una
  // caja aparte, puesta justo debajo del boton que arranca la prueba. No se mide nada
  // nuevo: es el unico escritor de rf-quality (pintarEnlace) leido dos veces.
  //
  // LOS TRAMOS Y LOS UMBRALES SON LOS QUE YA USA LA APP -RF_BIEN, RF_JUSTO y
  // ENLACE_ROTULO, arriba-, no una escala nueva: CLAUDE.md 14 prohibe una cifra que
  // esta app no pueda recalcular, y estas SI las recalcula, porque son las mismas
  // constantes de este mismo fichero.
  function pintarCajaAlcance(lectura) {
    if (!alcanceCajaEl) return;
    if (state.modo !== 'ALCANCE') {
      alcanceCajaEl.hidden = true;
      return;
    }
    alcanceCajaEl.hidden = false;
    const tramo = clasificarEnlace(lectura);
    const medido = tramo !== 'SIN_DATO';
    if (alcanceCajaPctEl) {
      alcanceCajaPctEl.textContent = medido ? lectura.pct + '%' : '--';
    }
    const hayRtt = lectura && lectura.rtt !== null && lectura.rtt !== undefined;
    if (alcanceCajaRttEl) {
      alcanceCajaRttEl.textContent = hayRtt ? 'RTT ' + lectura.rtt + ' ms' : 'RTT --';
    }
    if (alcanceCajaFraseEl) {
      alcanceCajaFraseEl.textContent = medido
        ? ENLACE_ROTULO[tramo] + ' · cuanto más alto, mejor: ' + RF_BIEN +
          '% o más es ' + ENLACE_ROTULO.BIEN.toLowerCase() + ', por debajo de ' +
          RF_JUSTO + '% está ' + ENLACE_ROTULO.CAYENDO.toLowerCase() + '.'
        : 'esperando la primera medida del equipo en esta prueba.';
    }
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
      // Se pregunta porque esto BORRA LA PRUEBA de una caida que a lo mejor no ha visto nadie (js/dialogo.js).
      Dialogo.confirmar('¿Vaciar la bitácora del enlace?', 'Se borra la guardada en este teléfono. ' +
                        'Si no la ha exportado, se pierde.', 'Vaciar la bitácora', () => {
        RegistroEnlace.limpiar();
        state.rfTramo = null;
        state.rfUltimaMuestraMs = null;
        renderRegistroEnlace();
        addEvent('cyan', 'Bitacora del enlace borrada a peticion del usuario.');
      });
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

  // ---- El diario de ordenes ----------------------------------------------
  //
  // La vista, la exportacion y los tres botones del diario viven en js/diario_vista.js
  // (corte del 28/09, sin cambio de comportamiento). Aqui queda renderDiario() como
  // envoltorio: la llaman el envio de ordenes, la rama de $ACK, las pestanas y el
  // arranque. Las funciones que se le pasan son declaraciones, y existen ya aqui.
  const diarioVista = DiarioVista.instalar({
    state: state, showToast: showToast, addEvent: addEvent, horaDe: _horaDe
  });

  function renderDiario() {
    diarioVista.render();
  }

  Exportar.conectarBoton(document.getElementById('btn-exportar-todo'), {
    vacio: () => !DiarioOrdenes.todas().length && !RegistroCrudo.todas().length,
    texto: () => diarioVista.texto() + '\n\n' + '='.repeat(72) + '\n\n' + textoDepuracion(),
    nombre: () => Exportar.nombreFichero(state.serie, new Date()), showToast, addEvent
  });

  // =========================================================================
  // 2. RENDER DE SEMAFOROS: SE PINTA LA PUNTA QUE HABLA, Y SE DECLARA LA OTRA
  // =========================================================================
  // Las tablas (ESTADOS, ESC_LUZ, MODOS, PLUMA_LEYENDA, CAM_LEYENDA) viven en js/luces_tablas.js y la
  // pintura (renderLights, pintarBadgeModo, pintarPluma, pintarCamaras) en js/luces.js: corte del 15/10
  // sin cambio de comportamiento (SPEC_4 9.12 tanda 2, D-98). Aqui quedan los envoltorios de siempre.
  const pintorLuces = Luces.instalar({
    state: state, s1Red: s1Red, s1Amber: s1Amber, s1Green: s1Green, s1Text: s1Text, s2Red: s2Red, s2Amber: s2Amber,
    s2Green: s2Green, s2Text: s2Text, phaseDescEl: phaseDescEl, plumaEl: plumaEl,
    badgeModoEl: badgeModoEl, camEstadoEl: camEstadoEl, camDetalleEl: camDetalleEl
  });

  function renderLights() {
    pintorLuces.renderLights();
  }

  function pintarPluma() {
    pintorLuces.pintarPluma();
  }

  function pintarCamaras() {
    pintorLuces.pintarCamaras();
  }

  function updateCountdownRing() {
    if (!cdNumEl || !ringProgressEl) return;
    // NUMERO O "--", Y NUNCA UN 0 DE RELLENO. `state.countdown` vale null mientras
    // ningun equipo haya dicho cuanto falta -al arrancar, y siempre que se hable con el
    // Esclavo, que manda `T:--` en todas sus tramas-. El HTML ya nace con "--" en
    // #cd-num: aqui solo hay que no estropearlo escribiendo una cifra inventada.
    const conocido = typeof state.countdown === 'number' && !Number.isNaN(state.countdown);
    const current = conocido ? Math.max(0, state.countdown) : 0;
    cdNumEl.textContent = conocido ? String(current) : (state.countdownImposible ? '!' : '--');

    const circumference = 251.32;
    // LA FRACCION SOLO SE DIBUJA SI HAY UN TOTAL QUE VENGA DE FUERA DE ESTA APP.
    //
    // countdownMax no llega en $STATUS -la trama trae T: y nada mas-, asi que el
    // unico que lo rellenaba era el formulario de tiempos de la propia app: el anillo
    // dibujaba una fraccion contra un total que se habia inventado el telefono. Sin
    // total conocido se deja el arco vacio, que es lo que se sabe; el numero de
    // segundos si viene del equipo y se sigue mostrando.
    // Sin cifra conocida tampoco hay fraccion que dibujar: un arco lleno junto a un
    // "--" seria el anillo afirmando lo que el numero acaba de negar.
    const total = (conocido && state.countdownMax > 0) ? state.countdownMax : 0;
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
    const clase = conocido && info ? info.anillo : 'sindato';   // D-83: '--' es gris, no un color de accion
    ringProgressEl.className = 'ring-fill ' + clase;
    cdNumEl.className = 'ring-num ' + clase;
  }

  // =========================================================================
  // 2.quater LA HORA DEL EQUIPO, QUE LLEGABA DESDE SIEMPRE Y NO LA LEIA NADIE
  // =========================================================================
  // N-150 (05/09). MEDIDO CON UN CENSO, no reportado desde el campo: `state.hora` tenia
  // un solo escritor -la rama de $STATUS- y NINGUN lector en app.js, en js/*.js ni en
  // index.html. El campo HORA: viaja en cada trama de las dos puntas desde hace meses y
  // la app lo guardaba para nada. Es exactamente la forma de CLAUDE.md 3.bis -la funcion
  // declarada, documentada y sin un solo llamador- aplicada a un DATO en vez de a una
  // funcion, y con el mismo final: se paga cuando alguien necesita mirarlo y no esta.
  //
  // Y esta noche empieza a costar de verdad: el puente ESP32 ya sella la hora del DS3231
  // en la trama, asi que HORA: deja de ser "--:--:--" y trae hora buena. Un dato que
  // acaba de empezar a existir y que ninguna pantalla ensena es un dato que no existe.
  //
  // LAS TRES RESPUESTAS SON DISTINTAS Y NINGUNA SE INVENTA NADA (CLAUDE.md 3.quinquies):
  //
  //   sin datos      el equipo no ha hablado, o el enlace se cayo y esta hora ya
  //                  envejecio. Un reloj parado pintado como si corriera es el peor de
  //                  los datos viejos: los demas se quedan quietos y este MIENTE
  //                  moviendose... o peor, no moviendose mientras el mundo avanza.
  //   SIN HORA       el equipo dice "--:--:--", que es lo que escribe su firmware cuando
  //                  reloj_enHora() es falso (Maestro/src/bluetooth.cpp). NO es un fallo
  //                  de la app ni un hueco: es el equipo declarando que no tiene hora, y
  //                  de esa hora cuelga la autorizacion del Modo Degradado.
  //   HH:MM:SS       la hora de su DS3231, tal cual vino. No se reformatea ni se compara
  //                  con la del telefono aqui: el boton de debajo es el que sincroniza y
  //                  quien decide es el que lee las dos.
  //
  // Lo que NO se hace es dar la hora por buena porque tenga forma de hora: un valor que
  // no case con HH:MM:SS se ensena EN CRUDO, igual que un MODO desconocido en el badge.
  function pintarHoraEquipo() {
    if (!equipoHoraEl) return;
    const decir = (txt, color) => {
      equipoHoraEl.textContent = txt;
      equipoHoraEl.style.color = color;
    };
    if (state.hora === null || state.hora === undefined) {
      decir('sin datos del equipo', 'var(--text-muted)');
      return;
    }
    const crudo = String(state.hora).trim();
    if (crudo === '00:00:00') {
      decir('00:00:00 · NO SINCRONIZADO', 'var(--amber-lamp)');
      return;
    }
    if (/^\d{2}:\d{2}:\d{2}$/.test(crudo)) {
      decir(crudo, 'var(--green-lamp)');
      return;
    }
    // Los guiones del firmware, y el vocabulario de ausencia que ya usan RF, RTT y BAT.
    // Se leen igual porque significan lo mismo, que es la razon de que esa lista sea una
    // sola (RF_NO_MEDIDO).
    if (/^-+:-+:-+$/.test(crudo) || RF_NO_MEDIDO.indexOf(crudo.toUpperCase()) >= 0) {
      decir('SIN HORA PUESTA · hay que sincronizar', 'var(--amber-lamp)');
      return;
    }
    decir('valor no reconocido: "' + crudo + '"', 'var(--text-muted)');
  }

  // =========================================================================
  // 3. ROL VISIBLE: LO DA LA CLAVE DE ADMINISTRACION, NO EL PIN (SPEC_4 9.2, D-56; js/clave_admin.js)
  // =========================================================================
  // Solo ensena u oculta. Las ordenes las autoriza el PIN del equipo (state.pinVerificado), que este rol no arma.
  function setRole(newRole) {
    state.role = newRole;
    document.body.classList.toggle('admin', newRole === 'TECNICO');
    if (newRole === 'TECNICO') {
      if (btnToggleRole) btnToggleRole.classList.add('admin');
      if (roleIconEl) roleIconEl.textContent = '🛡️';
      if (roleLabelEl) roleLabelEl.textContent = 'Técnico';
      adminTabs.forEach(t => t.style.display = 'flex');
      showToast('🛡️ Clave de administración: Tiempos, Diagnóstico y Depuración a la vista');
      addEvent('cyan', 'Sesion: clave de administracion puesta. Las ordenes siguen pidiendo el PIN del equipo.');
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
        ClaveAdmin.pedir(() => setRole('TECNICO'));
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
  // 3.bis LA AUTORIZACION CADUCA: 60 s DE GRACIA EN SEGUNDO PLANO, 5 min SIN MANDAR
  // =========================================================================
  // MEDIDO el 04/09: `state.pinVerificado` se ponia a `true` en UNA linea -validatePin()-
  // y no se apagaba en NINGUNA. No es una duracion larga: es que no habia duracion. El
  // arnes de DOM tuvo que montar un navegador entero para poder medir una sesion sin
  // autorizar, y dejo la frase escrita: "hoy dura lo que dure el proceso". En la calle
  // eso significa que el operario teclea, se guarda el telefono, y el siguiente que lo
  // coja manda ordenes que mueven luces sin teclear nada.
  //
  // SON DOS REGLAS Y HACEN FALTA LAS DOS, porque cada una cubre lo que la otra no ve:
  //
  //   SEGUNDO PLANO CON 60 s DE GRACIA. Salir de la app es soltarla. La gracia no es
  //   una concesion: el funcional reporta por WhatsApp desde el mismo telefono y sin
  //   ella estaria re-tecleando el PIN cada dos por tres, que es como se acaba
  //   escribiendo la clave en el capo con un rotulador.
  //
  //   5 min SIN MANDAR NINGUNA ORDEN. Cubre el caso que la anterior no ve: el telefono
  //   encendido y olvidado encima del capo, con la app delante. Ahi no hay ningun
  //   `visibilitychange` que salte nunca.
  //
  // Y lo que cuenta como actividad es HABER MANDADO, no haber tocado: el sello vive en
  // enviarComandoFirmware(), no en un oyente de la pantalla.
  const PIN_GRACIA_FONDO_MS = 60 * 1000;
  const PIN_INACTIVIDAD_MS = 5 * 60 * 1000;

  // Se dice con un toast y no con un modal a proposito. Un dialogo que hay que cerrar
  // se planta delante de un operario que puede estar en mitad de una maniobra; lo que
  // hay que hacer -volver a teclear- ya se lo va a pedir el propio boton en cuanto lo
  // pulse. La caducidad se AVISA, no se cobra con un paso mas.
  function caducarPin(motivo) {
    if (!state.pinVerificado) return;
    state.pinVerificado = false;
    // Los tres restos que closeModal() ya limpiaba al cerrar el teclado se limpian
    // tambien aqui: unos digitos buenos que sobrevivan a la caducidad la convierten en
    // un adorno -bastaria con un OK para rearmar la sesion sin teclear-.
    state.pin = '';
    state.accionPendiente = null;
    state.confirmacionVia = null;
    state.accionViaPendiente = null;
    state.ultimaOrdenMs = null;
    updatePinDisplay();
    if (pinModal) closeModal(pinModal);
    showToast('Autorizacion caducada: hay que teclear el PIN otra vez');
    addEvent('cyan', 'Autorizacion caducada (' + motivo + '). La siguiente orden que ' +
                     'lo necesite volvera a pedir el PIN.');
  }

  // Irse a segundo plano solo APUNTA LA HORA. Quien decide es la vuelta: mientras la
  // app no este delante, nadie puede mandar nada, asi que caducar durante la ausencia
  // no protege de nada y ademas obligaria a un temporizador que el sistema operativo
  // puede no dejar correr.
  function marcarFondo() {
    if (state.ocultaDesdeMs === null) state.ocultaDesdeMs = Date.now();
  }

  function volverDelFondo() {
    if (state.ocultaDesdeMs === null) return;
    const fuera = Date.now() - state.ocultaDesdeMs;
    state.ocultaDesdeMs = null;
    if (fuera > PIN_GRACIA_FONDO_MS) {
      caducarPin('la app estuvo ' + Math.round(fuera / 1000) + ' s en segundo plano');
    }
  }

  // Los dos sucesos, y no uno: `visibilitychange` es el que dispara al cambiar de app
  // dentro del sistema, y `pagehide` cubre la salida que no lo emite -el navegador que
  // congela la pestana, la vuelta atras del sistema-. Apuntar dos veces no hace dano
  // porque marcarFondo() no pisa el sello que ya haya.
  document.addEventListener('visibilitychange', () => {
    if (document.hidden) marcarFondo(); else volverDelFondo();
  });
  window.addEventListener('pagehide', marcarFondo);
  window.addEventListener('pageshow', volverDelFondo);

  function vigilarAutorizacion() {
    if (!state.pinVerificado || state.ultimaOrdenMs === null) return;
    // Con la app en segundo plano el reloj de inactividad no corre: esa ausencia ya la
    // juzga la regla de los 60 s al volver, y contarla dos veces caducaria a los 60 s
    // una sesion que la gracia acababa de perdonar.
    if (state.ocultaDesdeMs !== null) return;
    if (Date.now() - state.ultimaOrdenMs > PIN_INACTIVIDAD_MS) {
      caducarPin('pasaron ' + (PIN_INACTIVIDAD_MS / 60000) + ' min sin mandar ninguna orden');
    }
  }

  // =========================================================================
  // 3.ter LO QUE ABRE PASO NO SE GUARDA CON UN PIN: SE GUARDA MIRANDO LA VIA
  // =========================================================================
  // DECISION DEL RESPONSABLE, 04/09, y el porque hay que respetarlo al leer el codigo:
  //
  //   EL EQUIPO NO SABE SI QUEDAN VEHICULOS EN EL TRAMO. EL OPERARIO SI.
  //
  // Un PIN demuestra QUIEN eres. No demuestra que hayas MIRADO. Para una orden que
  // ABRE paso, la barrera util es la segunda, no la primera: el dato que falta no esta
  // en el telefono ni en el poste, esta en la calzada y solo lo tiene el que esta de
  // pie delante. Asi que las dos ordenes que abren paso desde la botonera de campo
  // dejan de pedir clave y pasan a pedir una respuesta sobre la via.
  //
  // El PIN NO se retira: sigue entero para el tecnico -tiempos, modos, reloj, test de
  // focos, reiniciar reloj- y sigue viajando en la trama. El firmware no cambia por
  // esto: lo pone la app.
  //
  // LAS TRES REGLAS QUE IMPIDEN QUE LA PREGUNTA SE VUELVA INVISIBLE, que son la parte
  // que de verdad decide si esto sirve para algo:
  //
  //   1. SOLO EN LO QUE ABRE PASO. Poner rojo y volver al menu NO preguntan nunca. Es
  //      la direccion segura, y es el mismo criterio que el firmware ya usa para el
  //      PIN -SIN_PIN incluye FORZAR_ROJO a proposito-. Preguntar para PARAR ensena a
  //      decir que si sin leer, y el dia que la pregunta llegue en serio ya no se lee.
  //
  //      🔴 Y AQUI ESTABA ESCRITO "PONER AMBAR" EN ESA MISMA LISTA. ERA FALSO, Y LO
  //      CORRIGE N-148 (05/09) A PETICION DEL RESPONSABLE: "cuando le das en manual en
  //      ambar, deberia salirte un aviso diciendo: ¿esta usted seguro que no hay
  //      vehiculos en el corredor?".
  //
  //      No es una preferencia suya contra una regla nuestra: la regla estaba mal
  //      clasificada, y se mide en el C++, no se opina. SET_MODO:AMBAR llama a
  //      modo_ambar_setup() (Maestro/src/modo_ambar.cpp), que hace DOS cosas:
  //      semaforo_iniciarFallo() en ESTA punta y protocolo_enviarPaquete(CMD_GO_AMBAR)
  //      a la otra. O sea AMBAR INTERMITENTE EN LOS DOS EXTREMOS a la vez -es el mismo
  //      final que el firmware describe en su propio acuse de AMBAR_EMERGENCIA:
  //      "talanquera ABIERTA: no es un rojo, los dos sentidos pasan con precaucion"-.
  //
  //      En un carril unico de 1000 m con un poste en cada punta, eso NO para el
  //      trafico: lo deja entrar por LOS DOS LADOS al mismo tiempo. De todas las
  //      ordenes de esta botonera es la que mas abre paso, no la que menos. DAR PASO
  //      abre un sentido; el ambar abre los dos.
  //
  //      Lo que sigue sin preguntar es la EMERGENCIA -AMBAR_EMERGENCIA del Esclavo y
  //      ROJO TOTAL del Maestro-, y ahi el motivo no es que no abra paso: es que se da
  //      viendo un accidente y una pregunta delante seria un paso mas entre el operario
  //      y la maniobra. La linea no la traza "abre o para", la traza "hay tiempo de
  //      mirar o no lo hay".
  //   2. LA PREGUNTA DICE QUE MIRAR, no "esta seguro". "Esta seguro" se contesta con el
  //      dedo; "mire hasta la curva" obliga a levantar la vista. Por eso el dialogo
  //      lleva la lista de lo que hay que mirar y el boton no se llama "Aceptar".
  //   3. NO SALE DOS VECES SEGUIDAS POR LO MISMO. Si acaba de contestar hace menos de
  //      30 s y el ciclo NO ha cambiado de fase, no se le vuelve a preguntar.
  //
  // Y una decision que va escrita porque no se ve en el codigo: se pregunta AUNQUE el
  // PIN este verificado. No son dos llaves de la misma puerta: el tecnico que tecleo su
  // clave hace diez minutos tampoco ha mirado el tramo.
  const VIA_VIGENCIA_MS = 30 * 1000;

  // La orden tal y como viaja por el cable, que es la unica forma de nombrarla que no
  // se puede confundir: SET_MODO:AUTO abre paso y SET_MODO:AMBAR no, y las dos llegan
  // aqui como el mismo `comando` con distinto `args`.
  function _ordenDeCable(comando, args) {
    return args ? comando + ':' + args : comando;
  }

  // QUE HAY QUE MIRAR, por maniobra. No es un rotulo: es la regla 2. Un texto generico
  // -"confirme la operacion"- se contesta sin levantar la vista, que es exactamente lo
  // que esta pregunta existe para impedir.
  const VIA_MANIOBRA = {
    'MANUAL:CAMBIAR_TURNO':
      'DAR PASO: el sentido que ahora tiene verde va a quedar en rojo y el otro va a ' +
      'arrancar. Mire el tramo entero antes de aceptar.',
    'SET_MODO:AUTO':
      'AUTOMATICO: el equipo va a empezar a dar verdes solo, sin volver a preguntar. ' +
      'Mire el tramo entero antes de aceptar.',
    // El texto NO dice "se pone en ambar", que es lo que el rotulo del boton ya dice y
    // no informa de nada. Dice lo que eso significa en la calzada, que es lo unico que
    // hace levantar la vista: los dos extremos en intermitente dejan entrar por los dos
    // lados a la vez, y en un carril unico eso es dos vehiculos de frente.
    'SET_MODO:AMBAR':
      'AMBAR: los DOS postes quedan en intermitente a la vez, asi que se podra entrar ' +
      'al corredor por las dos puntas. No es un rojo y no para a nadie. Mire el tramo ' +
      'entero antes de aceptar.'
  };

  // El vale sigue valiendo. Las TRES condiciones son la regla 3, y ninguna sobra:
  //
  //   - la MISMA orden. Haber mirado para dar paso no autoriza a arrancar el ciclo.
  //   - menos de 30 s. Un tramo de obra se llena en menos de un minuto.
  //   - la MISMA fase. Si el cruce cambio de luces desde que miro, lo que vio ya no es
  //     lo que hay -y tras un CAMBIAR_TURNO aceptado la fase cambia sola, asi que la
  //     segunda pulsacion vuelve a preguntar, que es lo correcto-.
  //
  // Sin telemetria `estadoLuces` vale null en los dos lados de la comparacion y la fase
  // deja de acotar: lo unico que queda estrechando la ventana son los 30 s. Va escrito
  // porque es una perdida real de la regla, no un detalle: sin equipo hablando la app
  // no puede saber que el cruce cambio.
  function viaConfirmadaVigente(orden) {
    const vale = state.confirmacionVia;
    if (!vale || vale.orden !== orden) return false;
    if (Date.now() - vale.ms > VIA_VIGENCIA_MS) return false;
    if (vale.fase !== state.estadoLuces) return false;
    return true;
  }

  // Devuelve si se puede seguir SIN preguntar. Si hay que preguntar, abre el dialogo y
  // deja la pulsacion en cola: se REPITE el click al confirmar, igual que hace
  // pedirPin(), en vez de ejecutar media accion desde dentro del dialogo. Repetir es lo
  // que garantiza que las barreras de arriba -la de punta- se vuelvan a evaluar con lo
  // que el equipo diga AHORA y no con lo que decia cuando se abrio el dialogo.
  function confirmarVia(orden, reintento) {
    if (viaConfirmadaVigente(orden)) return true;
    state.accionViaPendiente = reintento;
    state.ordenViaPendiente = orden;
    if (viaManiobraEl) {
      viaManiobraEl.textContent = VIA_MANIOBRA[orden] ||
        ('Esta orden ABRE paso: ' + orden + '. Mire el tramo entero antes de aceptar.');
    }
    openModal(viaModal);
    return false;
  }

  if (btnViaConfirmar) {
    btnViaConfirmar.addEventListener('click', () => {
      // Se recoge ANTES de cerrar, por el mismo motivo que validatePin(): cerrar el
      // dialogo cancela la cola, y esta accion no se cancela -se acaba de autorizar-.
      const accion = state.accionViaPendiente;
      const orden = state.ordenViaPendiente;
      closeModal(viaModal);
      if (!accion || !orden) return;
      // La fase se sella AQUI, en el instante en que el operario dice que miro. Sellarla
      // al mandar diria "la fase de cuando salio la orden", que es otra cosa.
      state.confirmacionVia = { orden: orden, ms: Date.now(), fase: state.estadoLuces };
      accion();
    });
  }

  if (btnViaCancelar) {
    btnViaCancelar.addEventListener('click', () => {
      closeModal(viaModal);
      addEvent('cyan', 'Orden cancelada en el aviso de via: no se dio paso.');
    });
  }

  if (modalViaClose) {
    modalViaClose.addEventListener('click', () => closeModal(viaModal));
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
  // un cruce parado para nada. Y el mismo orden vale para el aviso de via: pedirle que
  // mire el tramo para una orden que no se va a mandar es peor todavia, porque lo que
  // se gasta ahi no es tiempo, es la credibilidad de la pregunta.
  //
  // ESTOS DOS MANDOS YA NO PIDEN PIN: PIDEN LA VIA (04/09, ver 3.ter). Son los dos que
  // ABREN paso.
  //
  // N-148 (05/09): EL TERCERO -AMBAR- PIDE LAS DOS COSAS, Y EL ORDEN ESTA MEDIDO.
  //
  // Que tambien abre paso esta razonado arriba, en 3.ter. Lo que aqui hay que decidir es
  // si la via SUSTITUYE al PIN -como en los otros dos- o se SUMA. Se suma, y por dos
  // motivos que no son de estilo:
  //
  //   1. Retirarle el PIN a esta orden es una decision vial que nadie ha tomado. El
  //      responsable pidio ANADIR el aviso, no quitar la clave, y una orden que deja el
  //      cruce en intermitente por los dos lados no se afloja de paso.
  //   2. Es la unica puerta que teclea el PIN en el arnes de DOM. Ponerla detras de la
  //      via dejaria el bloque del "teclado fantasma" saliendo verde por el motivo
  //      equivocado -nadie confirmo el tramo- y el PIN dejaria de estar medido por
  //      nadie. Es CLAUDE.md 8.sexies: la barrera de abajo tapando a la de arriba.
  //
  // Y EL PIN VA DELANTE DE LA VIA, no al reves. El vale de via SELLA LA FASE en el
  // instante en que el operario dice que miro (viaConfirmadaVigente), asi que tiene que
  // ser lo ULTIMO antes de que la trama salga: con el teclado en medio, entre "he
  // mirado" y la orden caben los veinte segundos que se tarda en teclear cuatro
  // digitos con guantes, y en un tramo de obra eso es tiempo de sobra para que entre
  // un vehiculo.
  //
  // Lo que NO cambia y va escrito porque es una decision, no un olvido: se pregunta en
  // TODOS los modos, no solo en Manual. El responsable lo pidio para Manual -que es
  // desde donde se usa-, pero la orden es la misma desde AUTO o desde MENU y abre el
  // corredor igual. Colgar la pregunta de state.modo tendria ademas el defecto de
  // 3.septies al reves: sin telemetria state.modo vale null y la guarda dejaria de
  // preguntar justo cuando la app no sabe nada del cruce.
  if (btnOpAuto) {
    btnOpAuto.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO:AUTO');
      if (punta) { avisarOtraPunta('SET_MODO:AUTO', punta); return; }
      if (!confirmarVia('SET_MODO:AUTO', () => btnOpAuto.click())) return;
      // Se consume el bool en vez de apoyarse en la guarda de PIN de arriba, que ya no
      // esta: sin mirar lo que devuelve, la linea de abajo anunciaria "orden enviada"
      // de una orden que el emisor pudo no escribir (app_05_sin_exito_mudo).
      if (!enviarComandoFirmware('SET_MODO', 'AUTO')) return;
      addEvent('cyan', 'Operario: orden MODO AUTOMATICO enviada al equipo.');
    });
  }

  if (btnOpStep) {
    btnOpStep.addEventListener('click', () => {
      const punta = puntaCorrecta('MANUAL:CAMBIAR_TURNO');
      if (punta) { avisarOtraPunta('MANUAL:CAMBIAR_TURNO', punta); return; }
      if (!confirmarVia('MANUAL:CAMBIAR_TURNO', () => btnOpStep.click())) return;
      if (!enviarComandoFirmware('MANUAL:CAMBIAR_TURNO')) return;
      // N-162: aqui NO se dice "despejando": la orden acaba de SALIR y el equipo puede
      // contestar $ERR. Lo que el equipo hace lo dice su acuse (ACK_TEXTO). Y sin cifra
      // de segundos: el despeje es configurable y esta app no puede recalcularlo.
      addEvent('cyan', 'Operario: orden CAMBIAR TURNO enviada al equipo.');
    });
  }

  if (btnOpAmber) {
    btnOpAmber.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO:AMBAR');
      if (punta) { avisarOtraPunta('SET_MODO:AMBAR', punta); return; }
      if (!state.pinVerificado) { pedirPin(() => btnOpAmber.click()); return; }
      if (!confirmarVia('SET_MODO:AMBAR', () => btnOpAmber.click())) return;
      // Se consume el bool por lo mismo que en los dos de arriba: sin mirarlo, la linea
      // de abajo anunciaria "orden enviada" de una orden que el emisor pudo no escribir
      // (app_05_sin_exito_mudo).
      if (!enviarComandoFirmware('SET_MODO', 'AMBAR')) return;
      addEvent('cyan', 'Operario: orden MODO AMBAR enviada al equipo. Deja los DOS ' +
                       'postes en intermitente: espere el acuse, que dice si el ambar ' +
                       'acaba de entrar o si ya estaba y se ha vuelto a encender.');
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
      if (!enviarComandoFirmware('FORZAR_ROJO')) return;
      addEvent('red', TextosSpec4.emergenciaMaestro(null, state.modo === 'DEGRADADO').orden);
    });
  }

  if (btnOpAmbarEmergencia) {
    btnOpAmbarEmergencia.addEventListener('click', () => {
      if (state.node === 'MAESTRO') { avisarOtraPunta('AMBAR_EMERGENCIA', 'ESCLAVO'); return; }
      if (!enviarComandoFirmware('AMBAR_EMERGENCIA')) return;
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
      if (!enviarComandoFirmware('CANCELAR_AMBAR')) return;
      addEvent('cyan', 'Tecnico: orden RETIRAR AMBAR enviada al ESCLAVO. Espere el ' +
                       'acuse: el equipo dira si quedaba algun ambar y si queda otro ' +
                       'puesto desde la app.');
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
    const emM = TextosSpec4.emergenciaMaestro(punta, state.modo === 'DEGRADADO');
    if (emergenciaSubMaestroEl) emergenciaSubMaestroEl.textContent = emM.sub;
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
      emergenciaHintEl.textContent = emM.hint;
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
    [btnOpAuto, 'SET_MODO:AUTO'],
    [btnOpStep, 'MANUAL:CAMBIAR_TURNO'],
    [btnOpAmber, 'SET_MODO:AMBAR'],
    [btnOpMenu, 'SET_MODO:MENU'],
  ];

  function marcarDisponible(el, disponible) {
    if (!el) return;
    el.classList.toggle('no-disponible', !disponible);
    if (disponible) el.removeAttribute('aria-disabled');
    else el.setAttribute('aria-disabled', 'true');
  }

  function actualizarMandosDePunta(punta) {
    if (padPosteEl) {
      // Tres rotulos y no dos: "sin identificar" no es un poste, es la ausencia del dato, y
      // por eso tiene rotulo y color propios en vez de quedarse en blanco.
      padPosteEl.textContent = punta === 'MAESTRO' ? 'POSTE 1'
                             : punta === 'ESCLAVO' ? 'POSTE 2'
                             : 'Poste sin identificar';
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
    aplicarModoPantalla(punta);
  }

  // =========================================================================
  // 4.quater DOS PANTALLAS, PORQUE SON DOS COSAS
  // =========================================================================
  // DECISION DEL RESPONSABLE, 04/09: "al conectarnos al maestro, ese front debe
  // mostrarnos data de todo... y al conectarnos a una ventana de esclavo, sera para
  // diagnostico: logs de cuando conecta o no, y estado de las ordenes que recibe del
  // maestro. Ademas no puede permitir operar, pero si diagnosticar. La pantalla
  // confunde."
  //
  // Y la confusion tenia causa medible: esta pantalla intentaba ser las dos a la vez.
  // Contra el POSTE 2 ensenaba los CUATRO mandos de ciclo en gris -el despachador del
  // Esclavo no tiene esas ramas, ver SOLO_MAESTRO- y contra el POSTE 1 una columna
  // entera de SIN DATOS. Las dos mitades mienten por omision: una promete mandos que no
  // existen, la otra parece una averia del poste que calla.
  //
  //   POSTE 1 (MAESTRO)  consola de OPERACION: el ciclo, los tiempos, el modo.
  //   POSTE 2 (ESCLAVO)  ventana de DIAGNOSTICO: que le llega y que hace con ello.
  //
  // ✅ LO QUE HASTA EL 04/09 NO SE PODIA HACER, Y DESDE N-149 SI. Aqui estaba escrito
  // que la consola del Maestro NO podia ensenar las luces del Esclavo, porque el
  // $STATUS llevaba UN solo ESTADO -el del que lo manda- y el Maestro no publicaba el
  // del otro. Ya lo publica, en el campo ESC:, y renderLights() pinta las DOS columnas.
  //
  // El motivo de que se arreglara ahi y no aqui lo dio el responsable el 05/09, y es el
  // que hay que conservar: "hay un problema porque ahi esta haciendo la conexion por
  // Bluetooth, pero tendrias que caminar 1000 metros hasta el otro lado". Una consola
  // que para ensenar el otro extremo del cruce exige caminar hasta el no es una consola.
  //
  // El atajo "Conectarse al POSTE 2" SE QUEDA, y cambia lo que significa: ya no es el
  // unico modo de ver ese poste -- es el modo de abrir su ventana de DIAGNOSTICO, que
  // sigue teniendo cosas que ESC: no lleva (su bitacora, lo que le llega del Maestro).
  //
  // 🟠 LO QUE ESTA FUNCION NO DECIDE, Y VA ESCRITO PARA QUE LO DECIDA QUIEN DEBE: los
  // DOS MANDOS DE EMERGENCIA SE QUEDAN en la ventana del Esclavo. "No operar" se ha
  // leido como el CICLO -- que es lo que estaba en gris y lo que confunde --, no como la
  // parada de emergencia: AMBAR EMERGENCIA y RETIRAR AMBAR son las UNICAS ordenes que
  // el Esclavo atiende (SOLO_ESCLAVO), y retirarlas dejaria el ambar de emergencia de
  // ese poste sin ninguna via desde el telefono, que es la resolucion operativa de N-19
  // -- el tecnico ya no sube cinco metros de escalera --. Quitarlas es una decision
  // vial y es del responsable, no de esta linea.
  function aplicarModoPantalla(punta) {
    const enEsclavo = punta === 'ESCLAVO';
    const enMaestro = punta === 'MAESTRO';

    // Los cuatro mandos de ciclo DESAPARECEN contra el Esclavo en vez de quedarse en
    // gris. Un boton apagado sigue diciendo "esto se puede hacer aqui, pero ahora no", y
    // no es eso: en este poste no existe. Contra el Maestro y sin punta identificada
    // vuelven, porque ahi la respuesta si depende del momento.
    for (const par of MANDOS_DE_CICLO) {
      if (par[0]) par[0].style.display = enEsclavo ? 'none' : '';
    }
    if (padTituloEl) {
      padTituloEl.textContent = enEsclavo
        ? '🚨 EMERGENCIA DE ESTE POSTE'
        : '🎮 MANDOS DE CAMPO';
    }
    if (panelDiagnosticoEl) {
      panelDiagnosticoEl.style.display = enEsclavo ? '' : 'none';
    }
    // El atajo al OTRO poste sale en la columna que no tiene datos, y solo con una punta
    // CONFIRMADA: sin saber con quien se habla no se puede decir a donde ir.
    if (btnIrAlEsclavo) btnIrAlEsclavo.style.display = enMaestro ? '' : 'none';
    if (btnIrAlMaestro) btnIrAlMaestro.style.display = enEsclavo ? '' : 'none';
    // Al abrir la ventana se pinta lo que YA se sabe, sin esperar a la siguiente trama:
    // si no, el recuadro de la radio saldria con el cartel de "todavia no ha llegado
    // ninguna medida" durante hasta 30 s teniendo el dato guardado, y eso manda a buscar
    // una averia que no hay.
    if (enEsclavo) { renderDiagnostico(); renderDiagnosticoEnlace(); }
  }

  // El registro de la ventana de diagnostico. NO es una segunda copia de nada: lee
  // state.events, que es la misma lista que pinta la pestana de Eventos, y ahi ya entran
  // las conexiones y desconexiones (olvidarEnlace/abrirEnlace), los $ACK y $ERR con que
  // el equipo contesta, y los $EVENT con que cuenta lo que hizo por su cuenta. Escribir
  // aqui un registro propio seria la copia a mano que este repositorio ya pago tres
  // veces; lo unico que se hace es ACERCARLO, para no obligar a cambiar de pestana con
  // el poste delante.
  function renderDiagnostico() {
    if (!diagLogEl) return;
    diagLogEl.innerHTML = '';
    const ultimos = state.events.slice(0, 12);
    if (!ultimos.length) {
      const vacio = document.createElement('p');
      vacio.className = 'diag-log-vacio';
      // Vacio por no haber pasado nada y vacio por no estar mirando son cosas
      // distintas, y desde que se abrio la app solo puede ser lo primero.
      vacio.textContent = 'Sin movimientos desde que se abrió la app. Aquí van las ' +
                          'conexiones y desconexiones de este enlace, y lo que el equipo ' +
                          'conteste o avise por su cuenta.';
      diagLogEl.appendChild(vacio);
      return;
    }
    ultimos.forEach(ev => {
      const item = document.createElement('div');
      item.className = 'diag-log-item ' + (ev.type || '');
      const hora = document.createElement('span');
      hora.className = 'diag-log-hora';
      hora.textContent = ev.time;
      const txt = document.createElement('span');
      // textContent: dentro de estos mensajes van literales que llegan del equipo.
      txt.textContent = ev.msg;
      item.appendChild(hora);
      item.appendChild(txt);
      diagLogEl.appendChild(item);
    });
  }

  // D-23 - EL RECUADRO DE LA RADIO DE ESTE POSTE.
  //
  // Pinta el ULTIMO valor de cada contador y, al lado, LO QUE CAMBIO desde la muestra
  // anterior. Los dos juntos y no uno: el acumulado dice "cuanto lleva esta radio desde
  // que arranco" -que es lo que se cita al reportar- y el delta dice "que esta pasando
  // ahora mismo", que es lo que se mira con el poste delante. Con solo el acumulado, una
  // radio que se acaba de quedar muda ensena el mismo numero grande de siempre.
  //
  // TODO CON textContent: aqui dentro caen cifras que vienen del equipo por el cable.
  //
  // LO QUE ESTA FUNCION NO HACE ES DECIDIR SI ESTA BIEN O MAL. No hay clase de color por
  // umbral, porque el umbral no existe: ver la cabecera de js/diagnostico_enlace.js. El
  // unico rojo que se pinta es el del delta de bytes en CERO, y eso no es un umbral -es
  // la ausencia literal de trafico, "no entro nada", que no admite segunda lectura-.
  function renderDiagnosticoEnlace() {
    if (!diagRfDatosEl || !diagRfVacioEl) return;
    const resumen = DiagnosticoEnlace.resumen();
    if (!resumen) {
      // Sin muestra no se pinta un cero. Un 0 aqui diria "la radio no ha recibido nada",
      // que es un diagnostico durisimo; lo que pasa es que la medida no ha llegado, y son
      // cosas distintas. Es la misma regla de RegistroEnlace y la de ABORTADO/PASS.
      diagRfDatosEl.style.display = 'none';
      diagRfVacioEl.style.display = '';
      return;
    }
    diagRfVacioEl.style.display = 'none';
    diagRfDatosEl.style.display = '';

    const m = resumen.muestra;
    if (diagRfRxEl) diagRfRxEl.textContent = String(m.rx);
    if (diagRfOkEl) diagRfOkEl.textContent = String(m.ok);
    if (diagRfRuidoEl) diagRfRuidoEl.textContent = String(m.ruido);

    const d = resumen.delta;
    const seg = d ? Math.round(d.ventanaMs / 1000) : 0;
    // El texto del delta NOMBRA SU VENTANA. "+4" no significa nada sin saber en cuanto
    // tiempo; y la ventana se mide, no se supone que son 30 s: si el poste se salto una
    // muestra o el telefono estuvo en segundo plano, el hueco es mayor y hay que verlo.
    const pon = (el, v, sufijo) => {
      if (!el) return;
      el.textContent = d ? ('+' + v + ' en ' + seg + ' s' + (sufijo || '')) : 'primera medida';
    };
    pon(diagRfRxDEl, d ? d.rx : 0, '');
    pon(diagRfOkDEl, d ? d.ok : 0, '');
    pon(diagRfRuidoDEl, d ? d.ruido : 0, '');
    if (diagRfRxDEl) {
      diagRfRxDEl.className = 'diag-rf-delta' + (d && d.rx === 0 ? ' diag-rf-mudo' : '');
    }

    if (diagRfPieEl) {
      if (!d) {
        diagRfPieEl.textContent = 'Primera medida de este poste: los números son el ' +
          'total desde que arrancó su micro. La siguiente, en 30 s, ya dirá qué cambió.';
      } else if (d.rx === 0) {
        diagRfPieEl.textContent = 'En los últimos ' + seg + ' s NO entró un solo byte ' +
          'por esta radio. No es ruido: no está llegando nada.';
      } else {
        const pct = DiagnosticoEnlace.pctRuido(d);
        diagRfPieEl.textContent = 'En los últimos ' + seg + ' s entraron ' + d.rx +
          ' bytes. ' + (pct === null
            ? 'Ninguna trama completa se cerró en esa ventana: entran bytes sueltos.'
            : 'De las tramas que llegaron, ' + pct + ' % se descartaron.') +
          ' El juicio sobre si eso es aceptable es de quien está delante del poste: ' +
          'la app no fija ningún umbral.';
      }
    }
  }

  // EL CARTEL DE LA BARRERA RETENIDA, QUE ES LO QUE NO SE PUEDE DESALOJAR CONTANDO
  // LINEAS. El texto entero sale del modulo -titulo, medida, accion y limite-: aqui solo
  // se reparte en el DOM. Una frase escrita en esta funcion seria una segunda copia de
  // lo que el tecnico lee, y las suites miden la del modulo.
  function renderAvisoCamaraPluma() {
    if (!camPlumaEl) return;
    const v = (typeof AvisoCamaraPluma !== 'undefined') ? AvisoCamaraPluma.vigente() : null;
    if (!v) {
      camPlumaEl.hidden = true;
      return;
    }
    camPlumaEl.hidden = false;
    // La clase cambia el color, no el texto: quien lee sabe por el titulo si el brazo
    // sigue arriba, y el color solo acompana. Un cartel que se distinguiera SOLO por el
    // color no le dice nada a quien lo mira con el sol de frente.
    camPlumaEl.className = 'card card-camara-pluma' + (v.bajada ? ' camara-pluma-bajada' : '');
    if (camPlumaTituloEl) camPlumaTituloEl.textContent = (v.bajada ? '⚠️ ' : '⛔ ') + v.titulo;
    if (camPlumaMedidaEl) camPlumaMedidaEl.textContent = v.medida;
    if (camPlumaAccionEl) camPlumaAccionEl.textContent = v.accion;
    if (camPlumaLimiteEl) camPlumaLimiteEl.textContent = v.limite;
  }

  // N-170: poste y modo del $STATUS de ESTA conexion (olvidarEnlace() borra state.node).
  function renderAvisoCorte() {
    const c = (cortEl && typeof AvisoCorte !== 'undefined') ? AvisoCorte.cartel(
      state.node ? (state.node === 'ESCLAVO' ? 'POSTE 2 (ESCLAVO)' : 'POSTE 1 (MAESTRO)') : null,
      state.node && state.modo ? (MODOS[state.modo] ? MODOS[state.modo].texto : state.modo) : null) : null;
    if (!cortEl || (cortEl.hidden = !c)) return;   // sin cartel: oculto y fuera
    ['titulo', 'cuando', 'accion'].forEach(k => {
      document.getElementById('aviso-corte-' + k).textContent = (k === 'titulo' ? '⚠️ ' : '') + c[k]; });
  }
  const btnCorteOk = document.getElementById('btn-aviso-corte-ok');
  if (btnCorteOk) btnCorteOk.addEventListener('click', () => { AvisoCorte.cerrar(); renderAvisoCorte(); });
  function renderCarteles() { CartelesEquipo.pintar(document.getElementById('carteles-equipo'), renderCarteles); }   // js/carteles_equipo.js

  // VOLVER AL MENU: la salida de cualquier modo, sin PIN (ver SIN_PIN arriba).
  //
  // Ni aqui ni en ningun otro sitio se pinta "ya esta en el menu": desde el Degradado
  // el firmware NO cambia de modo al recibirla, contesta
  // $ACK,...,RESULT:SALIENDO_TODO_ROJO y tarda todavia el todo-rojo entero. Lo que el
  // equipo hizo lo dicen $ACK y $STATUS; esta app solo sabe que mando la orden.
  if (btnOpMenu) {
    btnOpMenu.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO:MENU');
      if (punta) { avisarOtraPunta('SET_MODO:MENU', punta); return; }
      if (!enviarComandoFirmware('SET_MODO:MENU')) return;
      addEvent('cyan', 'Operario: orden VOLVER AL MENU enviada al equipo.');
    });
  }

  // PRUEBA DE ALCANCE: rojo fijo mientras se mide el enlace. Sin PIN, como MENU.
  if (btnModoAlcance) {
    btnModoAlcance.addEventListener('click', () => {
      const punta = puntaCorrecta('SET_MODO:ALCANCE');
      if (punta) { avisarOtraPunta('SET_MODO:ALCANCE', punta); return; }
      if (!enviarComandoFirmware('SET_MODO:ALCANCE')) return;
      addEvent('cyan', 'Tecnico: orden PRUEBA DE ALCANCE enviada al equipo.');
    });
  }

  // D-37/D-46: el Degradado sin testigo ya no se ofrece (D-37) y su orden salio del
  // firmware (D-46); aqui vivian su manejador y su dialogo, sin boton desde el 29/09.
  // D-35 (testigo) y A-15 (degradado automatico) viven en js/testigo.js y js/deg_auto.js.
  Testigo.iniciar({ state, enviarComandoFirmware, puntaCorrecta, avisarOtraPunta, addEvent,
                    showToast, horaLocal24, fechaLocalISO, pedirPin });
  DegAuto.iniciar({ state, enviarComandoFirmware, addEvent, showToast, pedirPin, horaLocal24 });
  Noche.iniciar({ state, enviarComandoFirmware, addEvent, showToast, pedirPin });   // D-110, js/noche.js

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
  // El firmware inventa un literal distinto para cada final posible porque NO significan lo
  // mismo; pintarlos todos como "ACEPTADA (LITERAL)" deshace ese trabajo en la calzada. Que
  // ordenes tienen varios RESULT no se lista aqui: lo recuenta app_10 contra el C++.
  //
  // La tabla lleva los literales ESCRITOS ENTEROS para que el pack app_10 pueda cruzar
  // sus claves contra los literales del C++ en las dos direcciones: una respuesta que
  // el firmware manda y la tabla no nombra, y una que la tabla nombra y el firmware ya
  // no manda. Las dos hacen dano y no son la misma averia.
  // ---------------------------------------------------------------------------
  // D-15 - EL RELOJ DEL CRUCE SON DOS RELOJES, Y HASTA HOY LA APP ENSENABA UNO.
  //
  // EL DEFECTO, EN UNA LINEA: el tecnico ponia la hora en el poste que tenia delante,
  // leia "Hora puesta y propagada" y se iba creyendo que habia dejado el CRUCE en hora.
  // No habia dejado en hora mas que la mitad.
  //
  // POR QUE NO ES UN CAMPO NUEVO EN EL $STATUS, que era la otra salida posible. Medido,
  // no elegido:
  //
  //   1. NO HAY CAMINO. El DS3231 de cada poste cuelga de SU PROPIO ESP32, y ese ESP32
  //      solo habla con su STM32 (verbatim, y solo en el sentido app -> STM32: es B-1) y
  //      con el telefono que tenga delante. Para que el $STATUS del Maestro llevara el
  //      reloj del Esclavo, el dato tendria que ir DS3231(E) -> ESP32(E) -> STM32(E) ->
  //      radio -> STM32(M) -> ESP32(M) -> app. Eso cruza B-1, estrena un comando de radio
  //      y obliga al STM32 a publicar un campo de un reloj que no es suyo, que es
  //      EXACTAMENTE el error de categoria que D-15 acaba de quitar de en medio.
  //   2. NO CABE. El peor caso del $STATUS por tipo son 162 B contra un techo de 155, y
  //      ya no cabia antes de anadir nada (N-153). Un campo mas empeora un desbordamiento
  //      conocido, y la salida limpia es acotar RF y T donde se producen -no aqui-.
  //   3. NO HACE FALTA. El puente ya devuelve, por poste, la hora RELEIDA del chip en su
  //      $ACK (FECHA:/HORA:), y el $STATUS ya dice en NODE: a que poste esta conectada la
  //      app. Con esas dos cosas la comprobacion se hace aqui y cuesta CERO bytes de
  //      cable.
  //
  // Y DE PROPINA MIDE LO QUE NADIE MEDIA: el desfase entre los dos DS3231. No hay ningun
  // camino que los sincronice entre si -ni radio entre ESP32, ni nada que lleve la hora
  // de uno al otro-, asi que cada uno corre libre desde que alguien se la puso. El unico
  // instrumento posible es este: comparar las dos lecturas contra el reloj del telefono,
  // que es la referencia comun.
  //
  // 🛑 EL LIMITE, ESCRITO EN VEZ DE DISIMULADO: esto vive en memoria de la app, no en
  // disco. Aguanta lo que dura la caminata entre postes -que es para lo que existe-, y NO
  // sobrevive a que Android mate la app. Cuando falta un poste se dice "no consta en esta
  // sesion", que es lo unico cierto: la app no puede saber lo que no vio. No se inventa
  // persistencia sin haberla probado.
  const RelojDelCruce = {
    postes: {},

    // `poste` sale de NODE: del $STATUS. Si no se sabe a que poste se esta conectado NO
    // SE ANOTA NADA: atribuir la lectura al poste equivocado es peor que no tenerla,
    // porque produce un desfase inventado entre un reloj y el mismo reloj.
    anotar(poste, fecha, hora, telMs) {
      if (!poste || (poste !== 'MAESTRO' && poste !== 'ESCLAVO')) return false;
      const f = /^(\d{4})-(\d{2})-(\d{2})$/.exec(fecha || '');
      const h = /^(\d{2}):(\d{2}):(\d{2})$/.exec(hora || '');
      if (!f || !h) return false;
      const equipoMs = new Date(+f[1], +f[2] - 1, +f[3], +h[1], +h[2], +h[3]).getTime();
      if (!isFinite(equipoMs)) return false;
      this.postes[poste] = { fecha, hora, equipoMs, telMs };
      return true;
    },

    falta() {
      return ['MAESTRO', 'ESCLAVO'].filter(p => !this.postes[p]);
    },

    // EL DESFASE ENTRE LOS DOS DS3231, con el telefono de referencia comun.
    //
    // De cada poste se guarda el error contra el telefono en el instante de su acuse
    // -equipo menos telefono-, y se restan. Asi la caminata entre postes se cancela: da
    // igual que entre las dos lecturas pasen dos minutos o dos dias.
    desfaseSeg() {
      const m = this.postes.MAESTRO, e = this.postes.ESCLAVO;
      if (!m || !e) return null;
      return Math.round(((m.equipoMs - m.telMs) - (e.equipoMs - e.telMs)) / 1000);
    }
  };

  // ---------------------------------------------------------------------------
  // A-9 - LOS TRES RELOJES EN UNA SOLA LINEA DEL REGISTRO
  // ---------------------------------------------------------------------------
  // ES EL LECTOR QUE LE FALTABA A RelojDelCruce, y no una segunda copia de el.
  //
  // Lo que ya habia -anotar(), falta(), desfaseSeg()- lo estrena SET_RTC, o sea el
  // camino que ESCRIBE la hora. Para comprobar si el cruce esta en hora habia que
  // CAMBIARSELA, y con eso se pierde justo el dato que se buscaba: cuanto se habia
  // desviado. Desde LEER_RTC hay un camino que solo mira, y las dos puertas alimentan
  // la MISMA tabla: se restan lecturas de la misma sesion vengan de donde vengan.
  //
  // POR QUE SON TRES RELOJES Y NO DOS. El desfase entre los dos DS3231 se calcula
  // contra el del telefono, que es la referencia comun -y por eso la caminata entre
  // postes se cancela: da igual que pasen dos minutos o dos dias-. Pero esa referencia
  // TAMBIEN puede estar mal: un telefono con la hora automatica apagada, o en otra zona
  // horaria, deja los dos postes "desviados" por igual y el desfase entre ellos
  // perfecto. Si el celular no sale en el registro, quien lea el parte manana no puede
  // distinguir un cruce en hora de un cruce que coincide con un telefono equivocado.
  //
  // 🛑 EL LIMITE, ESCRITO EN VEZ DE DISIMULADO -es el mismo de RelojDelCruce y no se
  // repara aqui-: esta tabla vive en MEMORIA. Aguanta la caminata entre postes, que es
  // para lo que existe, y NO sobrevive a que Android mate la app. Por eso lo que falta
  // se dice "no consta en esta sesion" y no "el otro poste no se ha visto": la app no
  // puede saber lo que no vio, y las dos frases no significan lo mismo.
  function _relojLegible(p) {
    return p ? (p.fecha + ' ' + p.hora) : null;
  }

  // El parrafo del registro. Devuelve SIEMPRE las tres columnas -las que se saben y las
  // que no, dichas- y el desfase cuando se puede calcular. Un solo sitio que redacta,
  // para que las cuatro salidas no puedan divergir en el texto.
  function textoRelojDelCruce(poste, telMs) {
    // LA HORA SE COMPONE CON horaLocal24()/fechaLocalISO(), NO A MANO AQUI.
    //
    // Son las dos funciones que ya existen porque toLocaleTimeString() metio el reloj
    // 12 h tarde y toISOString() metio el dia de manana cada noche (seccion 1.bis). Un
    // segundo formateador escrito aqui seria una tercera copia del mismo acierto, y la
    // que se queda vieja el dia que alguien arregle las otras dos.
    const t = new Date(telMs);
    const cel = fechaLocalISO(t) + ' ' + horaLocal24(t);

    // EL QUE FALTA SE PREGUNTA A falta(), NO SE DEDUCE AQUI. Es la funcion que ya sabe
    // que postes tiene un cruce, y preguntarle mantiene una sola definicion: si manana
    // un cruce tuviera tres puntas, un `poste === 'MAESTRO' ? ... : ...` escrito aqui
    // seguiria diciendo que solo hay dos, en verde y sin que nada fallara.
    const otro = RelojDelCruce.falta()[0] ||
                 (poste === 'MAESTRO' ? 'ESCLAVO' : 'MAESTRO');
    const aqui = RelojDelCruce.postes[poste];
    const alla = RelojDelCruce.postes[otro];

    let s = 'Reloj del cruce · ' + poste + ' (conectado): ' + (_relojLegible(aqui) || '?');
    if (alla) {
      // LA LECTURA DEL OTRO POSTE VA CON LA HORA A LA QUE SE TOMO, y no es adorno: es
      // de otra visita. Sin ese sello, dos valores puestos uno al lado del otro se leen
      // como simultaneos, y entonces la diferencia CRUDA entre ellos parece el desfase
      // -que es justo el numero equivocado, porque incluye la caminata-.
      s += ' · ' + otro + ' (visto a las ' + horaLocal24(new Date(alla.telMs)) +
           '): ' + _relojLegible(alla);
    } else {
      s += ' · ' + otro + ': NO CONSTA EN ESTA SESION';
    }
    s += ' · celular: ' + cel;

    const d = RelojDelCruce.desfaseSeg();
    if (d === null) {
      s += '. FALTA ' + otro + ': conectese a ese poste y vuelva a consultar. Con un ' +
           'solo poste NO se ha validado el cruce, y esta lista no sobrevive a que ' +
           'Android cierre la app: si se cerro entre poste y poste, hay que repetir los dos.';
    } else {
      s += '. DESFASE MEDIDO entre los dos relojes del cruce: ' + d + ' s (cada poste ' +
           'contra el celular, restados: la caminata no cuenta).' +
           (Math.abs(d) <= 5 ? '' :
            ' REVISELO: son dos relojes independientes con pila propia y nada los ' +
            'vuelve a juntar solo.');
    }
    // NO SE DEVUELVE UN CAMPO `falta` QUE NADIE LEE. La ausencia del otro poste ya
    // esta DICHA en el texto y medida en `desfase === null`; un tercer campo con la
    // misma informacion es una copia mas que sincronizar y un lector menos que la mire.
    return { texto: s, desfase: d };
  }

  // LA PUERTA UNICA POR LA QUE SE ANOTA UNA LECTURA DE RELOJ, la escriba SET_RTC o la
  // lea LEER_RTC. Devuelve el texto que va al registro, o el motivo por el que no se
  // pudo anotar. Que las dos ordenes pasen por aqui es lo que garantiza que el desfase
  // se calcule igual venga de donde venga; dos copias serian dos formas de restar.
  function anotarLecturaDeReloj(data, rotulo) {
    const telMs = Date.now();
    if (!RelojDelCruce.anotar(state.node, data.FECHA, data.HORA, telMs)) {
      // El poste lo dice NODE: del $STATUS, NO este $ACK: el acuse viene marcado
      // NODE:PUENTE -es del ESP32, que es quien tiene el reloj- y eso identifica al
      // MODULO, no al POSTE. Sin $STATUS no se anota: una lectura atribuida al poste
      // equivocado fabrica un desfase entre un reloj y el mismo reloj.
      return { anotado: false, tono: 'amber',
               texto: rotulo + ': la app AUN NO SABE a que poste esta conectada (no ha ' +
                      'llegado ningun $STATUS con NODE). No se anota: repita cuando el ' +
                      'tablero identifique el equipo.' };
    }
    const r = textoRelojDelCruce(state.node, telMs);
    return {
      anotado: true,
      // Verde SOLO con los dos postes vistos y dentro de 5 s. Un poste solo no es un
      // cruce validado, asi que no se pinta como si lo fuera.
      tono: (r.desfase !== null && Math.abs(r.desfase) <= 5) ? 'green'
            : (r.desfase === null ? 'amber' : 'red'),
      texto: rotulo + ' · ' + r.texto
    };
  }

  // ACK_TEXTO vive en js/textos_ack.js; ERR_TEXTO y ERR_MOTIVO, en js/textos_err.js.
  // Corte del 14/10 sin cambio de comportamiento (SPEC_4 9.12, D-98): index.html los
  // carga antes que este fichero y aqui se consultan igual que antes.

  // Resuelve una entrada de las dos tablas de rechazo: puede ser el objeto directo o
  // una funcion de los campos de la trama, para los motivos que traen un dato dentro.
  function _traducirRechazo(data) {
    const entrada = DegAuto.rechazo(data) || Noche.rechazo(data) || DiagnosticoEnlace.acuseReinicios(data) ||
                    Testigo.rechazo(data, state.node) ||
                    TextosSpec4.ERR[(data.CMD || '?') + '|' + (data.DESC || '')] || ERR_TEXTO[(data.CMD || '?') + '|' + (data.DESC || '')] || ERR_MOTIVO[data.DESC || ''];
    if (!entrada) return null;
    return typeof entrada === 'function' ? entrada(data) : entrada;
  }

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
  // EL CUERPO DE ESTA FUNCION SE MUDO A js/nmea_parser.js EL 05/09, Y ESO ES EL ARREGLO.
  //
  // Habia DOS parsers con DOS convenios: este -claves verbatim, el que corre en el
  // telefono- y NMEAParser.parseStatus() -claves en minuscula y renombradas-, y de los
  // dos el que tenia pruebas unitarias era el que NO se instala. Un arreglo en aquel no
  // llegaba a la pantalla de nadie, y las suyas seguian en verde.
  //
  // Ahora hay UNO. El que gana es el convenio del cable -las claves tal cual viajan-,
  // por tres motivos medidos que estan escritos enteros en camposDeTrama(); el resumen
  // es que es el que corre, el que no lleva lista que sincronizar, y el que sirve a las
  // cinco cabeceras que esta app lee. parseStatus() sigue existiendo porque tiene un
  // consumidor de produccion fuera de la app -simulador_app_bluetooth.py, que corre en
  // la compuerta-, pero ya no PARTE la trama: la parte esta misma funcion.
  //
  // Se conserva el nombre local en vez de llamar a NMEAParser.camposDeTrama() en las
  // cinco ramas: un solo punto de entrada deja la mudanza en una linea y no en cinco, y
  // las ramas siguen leyendose igual. Lo que NO se conserva es una segunda copia de la
  // regla del primer ':'.
  function _camposNmea(parts) {
    return NMEAParser.camposDeTrama(parts);
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
    const esSello = Exportar.anotarVersion(state.firmware, header, _camposNmea(parts)); if (esSello) addEvent('green', Exportar.lineaVersion(_camposNmea(parts))); // antes de la cadena

    if (header === '$STATUS') {
      const data = _camposNmea(parts);

      // EL EFECTO DE UNA ORDEN SOLO LO SABE EL EQUIPO, Y LO CUENTA AQUI. Esta llamada
      // es la unica via por la que el diario se entera de si algo cambio: alimenta la
      // ventana de las ordenes abiertas y se queda como "lo que hay ahora", que sera el
      // "lo que habia antes" de la siguiente orden. Va lo primero, antes de pintar
      // nada, para que el instante que se apunta sea el de la trama y no el del render.
      DiarioOrdenes.verStatus(data, Date.now());

      // Cada $STATUS es la prueba de que el equipo sigue ahi. El watchdog de abajo la
      // usa para decidir si el tablero puede seguir mostrando lo que muestra.
      state.ultimoStatusMs = Date.now();
      if (!state.telemetriaViva) marcarConEnlace();

      if (data.NODE) {
        DegAuto.alStatus(data.NODE); Noche.alStatus(data.NODE);  // A-15 y D-110: consultan al identificar el poste
        state.node = data.NODE;
        if (nodeNameEl) {
          nodeNameEl.textContent = data.NODE === 'MAESTRO' ? 'MAESTRO (POSTE 1)' : 'ESCLAVO (POSTE 2)';
        }
        // La emergencia no es la misma maniobra en cada poste: el mando que se ensena
        // lo decide esta trama, no lo que el operario supone que tiene delante.
        actualizarEmergencia();
      }
      // N-62: SITE y PAIR no los emite ninguna punta (SITE es del gestor de cruces; PAIR espera a FW-PAIR).
      if (data.SERIE) {
        state.serie = data.SERIE; RegistroEquipo.peticion(state.serie, enviarComandoFirmware);  // D-75
      }
      // Se guarda lo que VENGA, incluido el vacio. Los dos switch del firmware tienen
      // salida por abajo -semaforo_nombreEstado() devuelve "" si el estado no es
      // ninguno de los cuatro; obtenerNombreModo() devuelve "DESCONOCIDO"-, y un
      // ESTADO vacio con el `if (data.ESTADO)` de antes NO se guardaba: la pantalla se
      // quedaba con el ultimo estado bueno pintado como si fuera de ahora. Un campo
      // que llega vacio es un dato -dice que el equipo no sabe-, no una trama sin
      // campo. renderLights() lo declara.
      if (data.MODO !== undefined) {
        state.modo = data.MODO;   // D-93: fuera de AMBAR se olvida su origen; un episodio nuevo no lo hereda
        if (data.MODO !== 'AMBAR' && typeof DiagnosticoEnlace !== 'undefined') DiagnosticoEnlace.olvidarAmbar();
        actualizarEmergencia();   // el texto del rojo total depende del modo (Degradado: solo este poste)
      }
      if (data.ESTADO !== undefined) {
        state.estadoLuces = data.ESTADO;
      }
      CartelesEquipo.alStatus(data.MODO); renderAvisoCorte(); renderCarteles();
      // N-149: EL ESTADO DEL OTRO POSTE, Y SE ASIGNA SIEMPRE -TAMBIEN CUANDO NO VIENE-.
      //
      // Los dos `if` de arriba solo escriben si el campo llego, y ahi es correcto: MODO
      // y ESTADO los emite CADA trama de las dos puntas, asi que "no vino" solo pasa con
      // una trama rota y conservar el anterior es lo razonable.
      //
      // Con ESC: no. Puede DEJAR de venir por dos caminos normales -se cambia de poste y
      // ahora habla un Esclavo, o el equipo de enfrente tiene firmware anterior a
      // N-149-, y en los dos el ultimo valor que se vio es de otro equipo o de otro rato.
      // Un `if (data.ESC !== undefined)` dejaria el semaforo del POSTE 2 pintado con ese
      // color como si fuera de ahora, que es exactamente 3.quinquies: lo que sustituye a
      // un dato que no se tiene no es un color, es decirlo.
      const escAntes = state.esc;
      state.esc = data.ESC !== undefined ? data.ESC : null;
      // N-153: LA PLUMA SE ASIGNA CON LA MISMA REGLA QUE ESC:, Y POR EL MISMO MOTIVO.
      //
      // Puede dejar de venir por un camino normal -el equipo de enfrente tiene firmware
      // anterior a N-153-, y en ese caso el ultimo valor visto es de otro rato. Un
      // `if (data.PLUMA !== undefined)` dejaria en pantalla una barrera ARRIBA que nadie
      // ha vuelto a confirmar, que es la peor de las tres salidas: el operario cruza
      // fiandose de una barrera que puede llevar bajada un minuto.
      const plumaAntes = state.pluma;
      state.pluma = data.PLUMA !== undefined ? data.PLUMA : null;
      // EL FIRMWARE NO PUBLICA UN EVENTO DE FIN DE LA RETENCION: deja de repetir el
      // aviso, y "dejar de repetir" es indistinguible de "se corto el Bluetooth". Quien
      // dice que el brazo ya bajo es este campo, que llega a cadencia fija. El cartel NO
      // se retira -lo que la dejo arriba sigue sin revisarse-, pero deja de afirmar que
      // la barrera esta arriba, que es lo que dejaria de ser cierto.
      if (typeof AvisoCamaraPluma !== 'undefined') {
        const solto = AvisoCamaraPluma.verPluma(state.pluma);
        if (solto) {
          renderAvisoCamaraPluma();
          addEvent(solto.tono, 'Equipo [CAMARA_PLUMA]: ' + solto.texto);
          RegistroEnlace.anotar('EVENTO', enlaceDeAhora(), solto.texto);
          renderRegistroEnlace();
        }
      }
      // D-13: LAS CAMARAS SE ASIGNAN CON LA MISMA REGLA QUE ESC: Y PLUMA:, Y POR EL
      // MISMO MOTIVO. El campo puede DEJAR de venir por un camino normal -el equipo
      // de enfrente tiene firmware anterior a D-13-, y un `if (data.CAM !== undefined)`
      // dejaria en pantalla un 'OK' que nadie ha vuelto a confirmar. Se pinta SIEMPRE,
      // aunque el campo no venga: pintarCamaras() sabe declarar la ausencia.
      state.cam = data.CAM !== undefined ? data.CAM : null;
      pintarCamaras();
      if (data.MODO !== undefined || data.ESTADO !== undefined || state.esc !== escAntes
          || state.pluma !== plumaAntes) {
        renderLights();
      }
      // El control de DEMANDA se abre y se cierra con el MODO que acaba de llegar, no
      // con lo que la app haya pedido: pedir Inteligente no es estar en Inteligente.
      if (data.MODO) {
        actualizarDemanda();
      }
      // N-150: EL CARTEL SE RETIRA CUANDO EL EQUIPO DICE QUE YA CICLA, no cuando la app
      // manda la orden de arrancar. Un $ACK de SET_MODO:AUTO solo dice que la orden se
      // acepto; el unico dato que vuelve FALSO el texto del cartel -"el cruce no esta
      // ciclando"- es que el propio equipo declare MODO:AUTO. Colgarlo del envio, o
      // incluso del acuse, dejaria el cartel retirado sobre un cruce que sigue parado
      // si la orden se perdio despues, que es la mitad silenciosa de este mismo defecto.
      if (data.MODO === 'AUTO') {
        mostrarArranqueTrasTiempos(false);
      }
      // N-139: `T:` YA NO ES UN NUMERO SIEMPRE, Y ESTA LINEA LO PINTABA COMO 0.
      //
      // El `|| 0` que habia aqui es el mismo defecto que el bloque de abajo documenta
      // haber quitado de RF, RTT y BAT. Con `T:--` -que el Esclavo manda SIEMPRE
      // (Esclavo/src/bluetooth.cpp:776) y el Maestro cuando no sabe cuanto falta
      // (Maestro/src/bluetooth.cpp:833)- parseInt da NaN y el `|| 0` escribia un CERO
      // en el anillo: "faltan 0 segundos, cambia ya", justo encima de un equipo que
      // acababa de decir que no lo sabe. Es §3.quinquies en un solo widget - lo que
      // sustituye a un dato que no se tiene no es una cifra, es decirlo.
      //
      // Y NO SE COLAPSA CON EL CERO LEGITIMO: `T:0` es el ultimo segundo de la fase y
      // significa lo contrario que `T:--`. Un anillo en 0 que quiera decir las dos
      // cosas no dice ninguna. Se guarda numero o null, y updateCountdownRing() sabe
      // pintar los dos.
      if (data.T !== undefined) {
        const seg = parseInt(data.T, 10);
        state.countdown = Number.isNaN(seg) ? null : seg;
        state.countdownImposible = String(data.T).trim() === '!';   // SPEC_4 5: `!` valor imposible, `--` no se sabe
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
      const lectura = lecturaDeEnlace(data); state.rfLectura = lectura;  // aviso_degradado.js
      pintarEnlace(lectura);
      // Mismo dato, misma trama: se pinta ademas en la caja de la Prueba de Alcance si
      // el equipo esta en ese modo (28/09). state.modo ya viene actualizado de arriba.
      pintarCajaAlcance(lectura);
      registrarMuestraEnlace(lectura); DegAuto.render();  // eleccion del automatico: botones y avisos sin radio
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
          batVoltageEl.textContent = state.battery === null ? '-- V' : state.battery.toFixed(1) + 'V';
          batVoltageEl.classList.toggle('green', state.battery !== null);   // verde solo con dato
        }
        if (batStatusEl) {
          batStatusEl.textContent = state.battery === null
            ? 'El equipo declara que no la mide: falta el divisor y la entrada analogica'
            : 'Medida del equipo';
        }
      }
      // N-150: SE GUARDA Y AHORA ADEMAS SE PINTA. Y la guarda pasa de `if (data.HORA)`
      // a `!== undefined` por el mismo motivo que ESTADO lo hizo unas lineas mas arriba:
      // con la truthy, un campo HORA que llegara VACIO no entraba y la pantalla se quedaba
      // con la hora anterior pintada como si fuera de ahora. Un campo que llega vacio es
      // un dato -dice que el equipo no supo rellenarlo-, no una trama sin campo.
      if (data.HORA !== undefined) {
        state.hora = data.HORA;
        pintarHoraEquipo();
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
      // D-26 (5): UNA ALARMA DICE QUE HACER, NO SOLO QUE PASO. La tabla vive en
      // js/avisos_equipo.js -el porque, en su cabecera- y sigue el molde de ERR_MOTIVO:
      // el literal en crudo se queda y la traduccion va detras. Lo que la tabla no
      // nombra sale como salia, en crudo, que es la red y no el destino.
      const aviso = Noche.alarma(data) || DegAuto.alarma(data) || AvisosEquipo.traducirAlarma(data);
      showToast(aviso && aviso.toast ? aviso.toast
                                     : 'ALERTA: ' + (data.EVENTO || 'Fallo detectado'));

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
      // La traduccion va PEGADA al literal y ANTES del tramo: lo que el tecnico tiene que
      // leer primero es que hacer, y el tramo es el dato para quien lo lea despues.
      const textoAlarma = 'Caja Negra: ' + (data.EVENTO || 'FALLO') + ' - ' +
                          (data.CAUSA || '') + ' (Accion: ' + (data.ACCION || '') + ')' +
                          (aviso ? ' -> ' + aviso.texto : '') +
                          (tramo.length ? ' [ultimo tramo: ' + tramo.join(', ') + ']' : '') +
                          ultimoVisto;
      addEvent(aviso && aviso.tono ? aviso.tono : 'red', textoAlarma);
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
      // LA RESPUESTA SE PEGA A SU ORDEN EN EL DIARIO, por el CMD que el firmware
      // devuelve. Lo que ese CMD garantiza esta censado en DiarioOrdenes.ALIAS_CMD y
      // CMD_SIN_ORDEN, con el grep que lo midio: casi todas vuelven con el literal
      // exacto, SET_TIEMPOS/SET_RTC vuelven sin argumentos y MANUAL:CAMBIAR_TURNO
      // vuelve como CAMBIAR_TURNO. Lo que no case NO se atribuye a ojo.
      DiarioOrdenes.verRespuesta('$ACK', data, line, Date.now());
      RegistroEquipo.verAck(data, addEvent);   // D-75: CMD:LOG del puente (no envia: lo hace el $STATUS)
      const cual = data.CMD || '?';
      const clave = cual + '|' + (data.RESULT || '');
      // D-15: EL ACUSE DEL RELOJ SE ANOTA POR POSTE, Y ES LO QUE CONVIERTE
      // "puse la hora aqui" EN "el cruce esta en hora".
      //
      // Se engancha al ACUSE y no al envio, por lo mismo que el mando de arrancar el
      // ciclo (N-150): lo unico que sabe si la hora quedo dentro es la respuesta, y
      // ademas el puente devuelve la hora RELEIDA del chip, no la que se mando.
      //
      // El poste lo dice NODE: del $STATUS, no este $ACK: el acuse viene marcado
      // NODE:PUENTE -es del ESP32, que es quien tiene el reloj- y eso identifica al
      // MODULO, no al POSTE. Si todavia no ha llegado ningun $STATUS no se anota nada y
      // se dice: una lectura atribuida al poste equivocado fabrica un desfase entre un
      // reloj y el mismo reloj.
      //
      // 🔴 Y DESDE A-9 SON DOS PUERTAS, NO UNA. Las dos anotan en la MISMA tabla, por
      // la MISMA funcion, y por eso una lectura de LEER_RTC en un poste se resta contra
      // una escritura de SET_RTC en el otro sin que nada tenga que saberlo. Lo que
      // cambia entre las dos es el rotulo -lo que el operario acaba de hacer-, no la
      // cuenta.
      const relojDeEsteAcuse =
        ((cual === 'SET_RTC' || cual === 'LEER_RTC') && data.FECHA && data.HORA)
          ? anotarLecturaDeReloj(
              data, cual === 'LEER_RTC' ? 'CONSULTA DE RELOJ' : 'Hora puesta')
          : null;
      // N-162: este acuse NO toca state.hora. Viene de NODE:PUENTE -el DS3231 del ESP32-
      // y state.hora es la hora del CONTROLADOR, la del $STATUS. Pintar una con la otra
      // tapaba justo lo que hay que ver: un STM32 que no recibio la hora.
      Testigo.anotarAcuse(data, state.node); Noche.anotarAcuse(data);   // D-110: su consulta va tras la del automatico
      const degAuto = DegAuto.acuse(data) || Noche.acuse(data) || DiagnosticoEnlace.acuseReinicios(data);  // A-15, D-64
      const dicho = ACK_TEXTO[clave] || degAuto || TextosSpec4.ACK[clave] || TextosSpec4.tiempos(clave, state.modo);
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
      // LA LINEA DE LOS TRES RELOJES VA LA ULTIMA, O SEA ARRIBA DEL TODO EN LA PANTALLA.
      //
      // addEvent() hace unshift, asi que el ULTIMO en anotarse es el PRIMERO que se lee.
      // El acuse generico dice lo mismo en cada consulta -"el modulo contesto"-; lo que
      // cambia, y lo unico que el operario ha venido a mirar, es el desfase. Se anota
      // despues para que sea eso lo que quede encima.
      //
      // Y LA ANOTACION EN SI YA OCURRIO ARRIBA, antes de pintar nada: si esto se moviera
      // a un `if` mas abajo que pudiera saltarse -un `return` nuevo entre medias-, la
      // lectura se perderia y el desfase se calcularia sin ella. Lo que se mueve es
      // DONDE SE PINTA, no donde se mide.
      if (relojDeEsteAcuse) addEvent(relojDeEsteAcuse.tono, relojDeEsteAcuse.texto);
      // N-150: EL MANDO PARA VOLVER A ARRANCAR SE ABRE AQUI Y NO AL PULSAR APLICAR.
      //
      // La diferencia es la de CLAUDE.md 6 leida desde este lado del cable: colgarlo
      // del envio abriria el cartel tambien cuando el equipo va a contestar
      // $ERR,DESC:RANGO o DESC:EN_MARCHA_PARE_EL_MODO -o cuando no contesta nada-, y
      // entonces estaria ofreciendo arrancar un ciclo con los tiempos VIEJOS mientras
      // el operario cree que son los que acaba de teclear. Se cuelga del acuse, que es
      // lo unico que sabe si los tiempos entraron.
      //
      // Y NO SE ARRANCA NADA DESDE AQUI: se ensena el mando. Arrancar el Automatico
      // ABRE PASO, y esa decision no la toma la app -"la maquina no decide sola operar
      // de un modo que nadie pidio"-. Lo que la app puede hacer, y hasta hoy no hacia,
      // es no dejar al operario deduciendo que le toca a el.
      if (data.CMD === 'SET_TIEMPOS' && data.RESULT === 'OK') {
        mostrarArranqueTrasTiempos(TextosSpec4.sinCiclo(state.modo));
      }
    } else if (header === '$EVENT') {
      // $EVENT es la bitacora del propio equipo -quien movio que y desde donde-. No la
      // leia nadie, asi que el registro de eventos de la app solo contenia lo que la
      // app misma habia hecho: una bitacora que no sabe nada de lo que pasa en el poste.
      const data = _camposNmea(parts);

      // D-23: el diagnostico periodico de radio (cada 30 s) se desvia a su recuadro, o
      // se comeria las 30 lineas de la bitacora; sus transiciones si van. El cartel de la
      // camara, igual: la cadencia refresca, solo empieza/se suelta escriben. Y son else,
      // no return: renderDepuracion() y renderDiario() tienen que repintar con esta trama.
      // N-170: el parte de arranque del puente (sin ORIGEN/DETALLE) va antes que nadie.
      RegistroEquipo.verParte(data);   // D-75: con LOG: arma la peticion del registro del puente
      if (RegistroEquipo.verRegistro(data, addEvent, enviarComandoFirmware)) { renderDepuracion(); return; }  // pasado
      const vistoCorte = (typeof AvisoCorte !== 'undefined') ? AvisoCorte.ver(data) : null;
      const vistoCam = (!vistoCorte && typeof AvisoCamaraPluma !== 'undefined')
        ? AvisoCamaraPluma.ver(data) : null; let vistoCartel = null;
      const esDiagRadio = !vistoCorte && typeof DiagnosticoEnlace !== 'undefined' &&
                          DiagnosticoEnlace.esPeriodico(data);
      if (vistoCorte) {
        if (!vistoCorte.repetido) {
          const textoCorte = 'Equipo [PUENTE]: EVT:ARRANQUE CAUSA:' + (data.CAUSA || '') +
                             ' ARRANQUES:' + (data.ARRANQUES || '') + ' -> ' + vistoCorte.texto;
          addEvent(vistoCorte.tono, textoCorte);
          if (vistoCorte.toast) showToast(vistoCorte.toast, 6000);
          RegistroEnlace.anotar('EVENTO', enlaceDeAhora(), textoCorte);
          renderRegistroEnlace();
          renderAvisoCorte();
        }
      } else if (vistoCam) {
        renderAvisoCamaraPluma();
        if (vistoCam.linea) {
          // El literal del equipo va DELANTE y la traduccion detras, que es el molde de
          // esta app: quien reporta necesita el DETALLE tal y como vino.
          const textoCam = AvisosEquipo.cabezaEvento(data) + ' -> ' + vistoCam.linea.texto;
          addEvent(vistoCam.linea.tono, textoCam);
          RegistroEnlace.anotar('EVENTO', enlaceDeAhora(), textoCam);
          renderRegistroEnlace();
        }
        if (vistoCam.toast) showToast(vistoCam.toast);
      } else if (esDiagRadio) {
        const visto = DiagnosticoEnlace.ver(data);
        if (visto && visto.pedirReinicios) enviarComandoFirmware('REINICIOS');  // D-64
        renderDiagnosticoEnlace(); if (visto && visto.ambarCambio) renderLights();  // D-93
        // LO QUE SI GASTA BITACORA SON LAS TRANSICIONES, NO LA CADENCIA: el ruido de las 03:40
        // queda escrito aunque nadie mirase. Dos por episodio como mucho -empieza y para-.
        if (visto && visto.avisos.length) {
          visto.avisos.forEach(a => {
            addEvent(a.tono, 'Equipo [radio de este poste]: ' + a.texto);
            RegistroEnlace.anotar('EVENTO', enlaceDeAhora(), a.texto);
          });
          renderRegistroEnlace();
        }
      } else if ((vistoCartel = CartelesEquipo.ver(data))) {   // js/carteles_equipo.js
        const textoCartel = vistoCartel.repetido ? null : AvisosEquipo.cabezaEvento(data) + ' -> ' + vistoCartel.texto;
        if (textoCartel) { addEvent(vistoCartel.tono, textoCartel); showToast(vistoCartel.toast, 6000); RegistroEnlace.anotar('EVENTO', enlaceDeAhora(), textoCartel); }
        renderCarteles(); renderRegistroEnlace();
      } else if (!esSello) {   // el sello EVT:VERSION ya salio en su linea "Version cargada"
      // D-26: tres lineas del diario que el tecnico tiene que entender -la siembra que
      // vuelve, la que se ignora porque manda la radio, y el salto de hora que pasa por
      // rojo-. Mismo molde que el $ALARM de arriba: crudo primero, traduccion detras, y
      // lo que la tabla no nombra en crudo y en cyan como siempre.
      const aviso = Noche.evento(data) || DegAuto.evento(data) || AvisosEquipo.traducirEvento(data);
      const textoEvento = AvisosEquipo.cabezaEvento(data) + (aviso ? ' -> ' + aviso.texto : '');
      addEvent(aviso && aviso.tono ? aviso.tono : 'cyan', textoEvento); if (aviso && aviso.toast) showToast(aviso.toast);
      RegistroEnlace.anotar('EVENTO', enlaceDeAhora(), textoEvento); renderRegistroEnlace();
      }
    } else if (header === '$ERR') {
      const data = _camposNmea(parts);
      // Igual que el $ACK: la negativa se pega a la orden que la provoco. Con una
      // salvedad medida y escrita en CMD_SIN_ORDEN: AUTH_FAILED y DESCONOCIDO NO
      // nombran la orden rechazada, asi que solo se atribuyen si quedaba una sola
      // esperando -y entonces se dice que fue por descarte-.
      DiarioOrdenes.verRespuesta('$ERR', data, line, Date.now());
      // Un rechazo del firmware NO se oculta: si el equipo dijo que no, la pantalla
      // tiene que decirlo tambien, o el operario se va creyendo que la orden entro.
      //
      // NO_HAY_AMBAR_VIGENTE se traduce porque es la tercera respuesta de R-3 y su
      // literal, leido tal cual, se confunde con una averia. No lo es: es el equipo
      // diciendo que no habia nada que quitar, y eso es una respuesta util -significa
      // que si el cruce sigue en ambar, viene de otro sitio-.
      // Quien rechaza importa tanto como por que: seis de los motivos del reloj los
      // emite el PUENTE (el ESP32), no el micro del semaforo, y quien va a destapar un
      // conector necesita saber cual de los dos modulos se esta quejando. El campo NODE
      // solo lo traen las tramas del puente; en las del STM32 no existe y no se inventa.
      const delPuente = data.NODE === 'PUENTE' ? ' (PUENTE, modulo ESP32)' : '';

      // EL LITERAL EN CRUDO NO SE SUSTITUYE POR LA TRADUCCION: VAN LOS DOS, Y EN ESE
      // ORDEN. La traduccion es para el que esta delante del poste ahora; el literal es
      // para el que lea esto despues -el registro se exporta a CSV y a WhatsApp, y
      // NO_QUEDO_PUESTA es el termino con el que se busca en el roadmap y con el que se
      // reporta-. Cambiar uno por otro habria ganado una frase util y perdido la unica
      // palabra que se puede citar.
      const cabecera = 'Rechazo de Firmware' + delPuente + ': [' + (data.CMD || '?') + '] ' +
                       (data.DESC || '');

      // A-9 - UN RELOJ QUE NO CONTESTA NO ES UNA COLUMNA VACIA: ES EL CRUCE SIN VALIDAR.
      //
      // Los siete literales de arriba dicen QUE le pasa a ESTE reloj. Lo que ninguno
      // puede decir -porque es un dato de la sesion, no de la trama- es la consecuencia
      // sobre el CRUCE: si el otro poste ya estaba anotado, el desfase se queda sin
      // calcular, y esa es justo la comprobacion que el operario venia a hacer.
      //
      // Y AQUI NO SE ANOTA NADA, que es la mitad que importa. La tentacion seria
      // guardar la lectura anterior de este poste "para no perderla": eso convertiria
      // una hora de hace media hora en la de ahora, y el desfase saldria calculado
      // sobre un dato que el equipo acaba de declarar no fiable. Es N-144 en la app.
      if (data.CMD === 'LEER_RTC') {
        const otro = RelojDelCruce.falta().filter(p => p !== state.node);
        addEvent('red',
          'CONSULTA DE RELOJ: este poste (' + (state.node || 'sin identificar') + ') NO ' +
          'ha entregado hora, asi que NO se anota nada y el desfase del cruce SIGUE SIN ' +
          'MEDIR' + (otro.length === 0 && state.node
            ? ' (el otro poste si consta en esta sesion, pero un desfase necesita los dos).'
            : ' (tampoco consta el otro poste en esta sesion).') +
          ' El motivo concreto va en la linea de al lado.');
      }

      const negado = RegistroEquipo.rechazo(data) || _traducirRechazo(data);   // D-75: los dos de CMD:LOG
      if (negado) {
        addEvent('red', cabecera + ' -> ' + negado.texto);
        showToast(negado.toast);
      } else {
        // FALLBACK, Y SE QUEDA A PROPOSITO -es la mitad que impide que la tabla de
        // arriba se convierta en un filtro-. Un motivo nuevo del firmware tiene que
        // llegar a la pantalla aunque nadie le haya escrito todavia su traduccion: en
        // crudo es feo y util; tragado es el "error" a secas del que venimos. Y se dice
        // que la app no lo sabe traducir, para que quien lo lea sepa que le falta app,
        // no que le falte equipo.
        addEvent('red', cabecera + ' (motivo sin traducir en esta version de la app)');
        showToast('Rechazado por el equipo: ' + (data.DESC || 'ver eventos'));
      }
    }
    // La cinta se repinta una sola vez por trama y al final, cuando los reparos ya
    // estan puestos: hacerlo arriba ensenaria la trama sin ellos y el usuario veria la
    // linea cambiar sola.
    renderDepuracion();
    // El diario tambien: una misma trama puede haberle puesto la respuesta a una orden
    // y el efecto a otras tres.
    renderDiario();
  }

  // Despachador unico de los botones que declaran su orden en data-cmd. Un solo sitio
  // que traduce el atributo a comando, para que el boton del HTML y lo que sale por
  // Bluetooth no puedan divergir: el pack app_01_comandos lee justo ese atributo.
  //
  // LAS DOS PUNTAS ACEPTAN CONJUNTOS DISTINTOS, y la app mandaba a ciegas. SET_MODO solo
  // lo entiende el Maestro (D-46 retiro SOLICITAR_PASO y SET_MODO:DEGRADADO del
  // Esclavo, Maestro/src/bluetooth.cpp): pulsados contra la punta equivocada
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
  // A-11 (05/09): 'SET_MODO' A SECAS SE ROMPE EN SUS SEIS, Y NO ES ORDEN NI ESTILO.
  //
  // La entrada cubria por raiz -'SET_MODO' gobierna las siete SET_MODO:X-, y eso era
  // correcto mientras el Esclavo no atendiera ninguna. Desde el 05/09 atendia UNA:
  // SET_MODO:DEGRADADO (A-11), que D-46 retiro de las dos puntas; al Degradado se entra
  // por el testigo. Las seis de abajo siguen siendo del Maestro.
  //
  // DEJARLA COMO ESTABA NO HABRIA SIDO "un poco estricto de mas": la app habria
  // contestado "esa orden la atiende el MAESTRO" delante de un Esclavo que SI la
  // atiende, o sea que el boton habria quedado inutil justo en el poste para el que se
  // construyo. Lo caza app_08_enrutado_por_punta, que recalcula el reparto de los dos
  // despachadores y prohibe que una lista reclame una orden que atienden las dos.
  const SOLO_MAESTRO = ['SET_MODO:AUTO', 'SET_MODO:MANUAL', 'SET_MODO:AMBAR',
                        'SET_MODO:MENU', 'SET_MODO:ALCANCE', 'SET_MODO:INTELIGENTE',
                        'MANUAL:CAMBIAR_TURNO', 'SET_TIEMPOS', 'TEST_LEDS',
                        'DEMANDA', 'REINICIAR_RELOJ', 'FORZAR_ROJO'];
  // CANCELAR_AMBAR (R-3) vive en el Esclavo, que tiene el latch como en AMBAR_EMERGENCIA:
  // contra un Maestro volveria como $ERR,CMD:DESCONOCIDO, el error que parece boton roto.
  const SOLO_ESCLAVO = ['AMBAR_EMERGENCIA', 'CANCELAR_AMBAR'];

  // 🟠 N-124, VENTANA CONOCIDA Y ABIERTA A PROPOSITO - NO ES UN DESCUIDO.
  // `SOLO_MAESTRO` pregunta `=== 'ESCLAVO'`: con `state.node` en null (el segundo entre que abre el socket y llega el
  // primer $STATUS) una orden de Maestro SI sale. Cerrarla con `!== 'MAESTRO'` rompio el arnes del puente, que pulsa
  // AUTOMATICO nada mas conectar; arreglarlo es que el arnes entregue un $STATUS antes (roadmap). Coste, acotado: ~1 s
  // por conexion con una orden SOLO_MAESTRO posible contra un Esclavo, que la rechaza con $ERR,CMD:DESCONOCIDO.
  // A-11: SE CONSULTA CON LA ORDEN ENTERA, no con su raiz, y la coincidencia es por PREFIJO DE SEGMENTO: la MISMA regla
  // que app_08_enrutado_por_punta usa para decidir si una entrada gobierna una orden (_cubre). 'SET_TIEMPOS' cubre
  // 'SET_TIEMPOS:30:5:20'; cortar por el primer ':' dejaria sin gobernar las seis SET_MODO:X (mandos de ciclo contra el
  // Esclavo sin aviso). Una regla distinta aqui seria un segundo contrato de enrutado.
  function puntaCorrecta(orden) {
    const cubre = e => orden === e || orden.indexOf(e + ':') === 0;
    if (SOLO_MAESTRO.some(cubre) && state.node === 'ESCLAVO') return 'MAESTRO';
    if (SOLO_ESCLAVO.some(cubre) && state.node !== 'ESCLAVO') return 'ESCLAVO';
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

      // A-11: se pregunta con la ORDEN ENTERA, no con su raiz. Con las seis entradas
      // SET_MODO:X escritas una a una en SOLO_MAESTRO, preguntar por 'SET_MODO' no
      // casaria con ninguna y los mandos de ciclo saldrian al cable contra un Esclavo
      // que no los atiende. `comando` sigue haciendo falta abajo, para el envio.
      const punta = puntaCorrecta(orden);
      if (punta) {
        avisarOtraPunta(orden, punta);
        return;
      }

      if (!state.pinVerificado) { pedirPin(() => btn.click()); return; }
      if (!enviarComandoFirmware(comando, args)) return;
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
        Dialogo.pedirTexto({ titulo: 'Renombrar el cruce', texto: 'Nombre nuevo:', valor: site.nombre }, (nombre) => {
          SiteManager.actualizarCruce(site.id, nombre, site.ubicacion, site.p1, site.p2);
          // Si el cruce editado es el activo, la cabecera cambia con el: un rotulo viejo es un cruce mal identificado.
          if (currentSiteNameEl && state.site && site.nombre.includes(state.site)) {
            state.site = nombre.replace('📍', '').trim();
            currentSiteNameEl.textContent = '📍 ' + state.site;
          }
          renderSiteList(siteSearchInput ? siteSearchInput.value : '');
          addEvent('cyan', 'Cruce renombrado a ' + nombre);
        });
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
      // Campos VACIOS (SPEC_4 9.1). Se activa en el acto: el rotulo de la cabecera es lo unico que le dice al
      // tecnico sobre que cruce manda ordenes, y quedarse en el anterior pide un paso que nadie recuerda.
      Dialogo.pedirTexto({ titulo: 'Nuevo cruce', texto: 'Nombre del cruce:' }, (nombre) => {
        const guardar = (ubicacion) => {
          const nuevo = SiteManager.agregarCruce(nombre, ubicacion);
          state.site = nuevo.nombre.replace('📍', '').trim();
          if (currentSiteNameEl) currentSiteNameEl.textContent = '📍 ' + state.site;
          renderSiteList();
          showToast(`✓ Cruce "${nuevo.nombre}" guardado.`);
        };
        Dialogo.pedirTexto({ titulo: 'Nuevo cruce', texto: 'Ubicación o PR (opcional):', opcional: true },
                           guardar, () => guardar(''));
      });
    });
  }

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
  // LO QUE ESTA APP NO PUEDE SABER, y va escrito para que nadie lo de por cubierto: por
  // que se cayo. El plugin entrega un texto fijo, no un motivo. Si el socket lo cerro el
  // sistema para dejar sitio a otra aplicacion, el aviso llega igual y no lo dice. La
  // causa no se inventa.

  // El estado del enlace se apaga SIEMPRE por aqui: "he desconectado" y "me han desconectado" no dejan la pantalla
  // en estados distintos (un boton que dice enlazado sin estarlo). D-86: js/practica.js pregunta y avisa por estas dos.
  window.appEnlazada = () => state.connected === true; window.appAvisar = (t, m) => { showToast(m); addEvent(t, m); };
  function olvidarEnlace() {
    state.connected = false;
    state.node = null;
    state.firmware = {}; renderLights();   // sin enlace: «SIN ENLACE», no la frase del equipo de antes
    // D-23: los contadores de radio son de UN poste y arrancan con su micro. Si no se
    // olvidan al soltar el enlace, la primera muestra del SIGUIENTE poste se resta contra
    // la ultima del anterior y sale un delta inventado -y casi siempre negativo, que
    // ademas se leeria como "este poste se ha reiniciado"-. Es la misma cautela que A-9
    // sobre el reloj: una lectura atribuida al poste equivocado fabrica una averia.
    if (typeof DiagnosticoEnlace !== 'undefined') {
      DiagnosticoEnlace.olvidar();
      renderDiagnosticoEnlace();
    }
    // Y por lo mismo el cartel de la barrera retenida: es de UN poste. Colgado sobre el
    // siguiente mandaria a mirar debajo del brazo que no es, y ademas dice "no va a
    // bajar sola" de una barrera que nadie ha vuelto a mirar.
    if (typeof AvisoCamaraPluma !== 'undefined') {
      AvisoCamaraPluma.olvidar();
      renderAvisoCamaraPluma();
    }
    if (typeof AvisoCorte !== 'undefined') { AvisoCorte.olvidar(); renderAvisoCorte(); } CartelesEquipo.olvidar(); renderCarteles();
    if (btnDevice) btnDevice.className = 'btn-top btn-device';
    if (btStatusDot) btStatusDot.className = 'status-dot';
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
        state.connected = true; renderLights();   // D-80: «esperando su primer dato» hasta el primer $STATUS
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
        Exportar.pedirVersionAlConectar(state, enviarComandoFirmware, addEvent, TIMEOUT_ENLACE_MS);
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
          showToast('❌ No se pudo conectar');
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
    state.ultimoEquipo = { mac: mac, name: name, emparejada: !!emparejada };

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
    return ListaEquipos.repintarEquipos(btDeviceListContainer, equiposVistos, notaLista);
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
  // 7.bis N-150: LOS TIEMPOS ENTRAN, EL CRUCE SE QUEDA EN ROJO, Y NADIE LO DECIA
  // =========================================================================
  // Reportado el 05/09 con el equipo delante: "le coloque 4 minutos y 15 seg, se cambia
  // el tiempo, se le da aplicar tiempos y se queda en rojo maestro y esclavo y no
  // cambian". Los cuatro minutos nunca llegaron a contar.
  //
  // NADA DE LO QUE PASO ESTABA ROTO, y por eso costo verlo. Para fijar tiempos hay que
  // sacar el equipo del ciclo -el firmware rechaza SET_TIEMPOS con
  // EN_MARCHA_PARE_EL_MODO mientras el Automatico corre-, y en el menu el coordinador
  // del Maestro manda CMD_GO_RED a las dos puntas cada 3 s: el cruce parado en rojo es
  // lo que el menu HACE, a proposito. Lo que faltaba es el ultimo paso, que es de una
  // persona: volver a arrancar el ciclo.
  //
  // POR QUE LA APP NO LO ARRANCA SOLA, que es la parte que hay que defender y no
  // esconder. Encadenar un SET_MODO:AUTO al $ACK de los tiempos es de una linea y es
  // exactamente lo que CLAUDE.md 6 prohibe: seria un cambio de modo que ABRE PASO sin
  // que nadie lo pulse, decidido por el telefono, sobre un tramo que nadie ha mirado.
  // El aviso de via que protege al boton AUTOMATICO de la botonera existe justo para
  // que esa orden no salga sin que alguien levante la vista; automatizarla lo saltaria
  // entero. "La maquina no decide sola operar de un modo que nadie pidio."
  //
  // Lo que SI es responsabilidad de la app, y era el hueco de verdad: no dejar que el
  // operario tenga que DEDUCIR que le toca a el. El cruce en rojo se explica, y el
  // mando para arrancarlo se pone donde esta mirando -debajo del formulario que acaba
  // de rellenar-, con su aviso de via detras como cualquier otra orden que abre paso.
  function mostrarArranqueTrasTiempos(visible) {
    if (!avisoTiemposParado) return;
    avisoTiemposParado.hidden = !visible;
  }

  if (btnArrancarCiclo) {
    btnArrancarCiclo.addEventListener('click', () => {
      // SE REENVIA AL BOTON DE LA BOTONERA EN VEZ DE REPETIR SU CUERPO, y no es
      // comodidad: es lo unico que garantiza que este mando pase por las MISMAS tres
      // barreras que el otro -la punta correcta, el aviso de via, y el bool de
      // enviarComandoFirmware()-. Copiar las cuatro lineas aqui crearia una segunda
      // puerta de salida para SET_MODO:AUTO, que es como se abren los agujeros que este
      // repositorio ya ha pagado: el dia que alguien anada una barrera a la botonera,
      // esta copia se quedaria sin ella y nada lo diria.
      //
      // Ademas mantiene el censo de app_01 midiendo lo que mide: el literal
      // 'SET_MODO','AUTO' sigue existiendo en UN solo sitio del fuente.
      if (!btnOpAuto) {
        showToast('No se encuentra el mando AUTOMATICO en esta pantalla');
        return;
      }
      btnOpAuto.click();
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
      // "Enviada", no "exitosa". Esta app no sabe si el Esclavo puso la hora hasta que
      // conteste: lo atiende el puente ESP32 de ese poste (NODE:PUENTE) y, si su reloj la
      // acepta, siembra a la controladora con la hora releida (D-26).
      showToast(`Orden enviada al Esclavo: ${today} ${comp.horaCompensada}`);
      addEvent('cyan', `Courier RTC: orden SET_RTC enviada (${today} ${comp.horaCompensada}, ` +
                       `traslado ${comp.elapsedSeg}s). Espere el acuse del equipo.`);

      if (btnCourierInject) btnCourierInject.disabled = true;
    });
  }

  // A-9 - LA CONSULTA. VA ANTES QUE EL BOTON QUE PISA LA HORA, y ese orden es el
  // arreglo: hasta hoy la unica forma de LEER el reloj era MANDARLO -el $ACK de SET_RTC
  // devuelve la hora releida del chip-, o sea que comprobar si el cruce estaba en hora
  // obligaba a cambiarsela, y con eso se perdia justo el dato que se buscaba.
  if (btnLeerRtc) {
    btnLeerRtc.addEventListener('click', () => {
      // NO SE PINTA NADA AQUI, y es lo mismo que N-150 con el arranque tras tiempos:
      // lo unico que esta funcion sabe es que la orden salio. Los tres relojes y el
      // desfase los escribe el ACUSE, que es lo unico que ha visto un chip.
      if (!enviarComandoFirmware('LEER_RTC')) return;
      showToast('Consulta de reloj enviada');
      addEvent('cyan', 'CONSULTA DE RELOJ enviada al puente de este poste. No cambia ' +
                       'nada: ni el reloj, ni una luz, ni un modo. Espere el acuse; si ' +
                       'no llega, este poste no tiene puente ESP32 detras del conector ' +
                       'y su hora no se puede consultar desde aqui.');
    });
  }

  // 1.51 - QUE FIRMWARE LLEVA ESTE EQUIPO.
  //
  // POR QUE ESTE BOTON EXISTE, medido en campo el 16/09: se exporto una cinta entera del
  // Maestro del Sisga -80 ordenes y 132 tramas- y NO habia forma de saber que firmware
  // estaba dentro. El hash solo existia en la memoria de quien cargo la tarjeta, y
  // CLAUDE.md 0.2 dice que una foto de campo sin ese hash no se lee.
  //
  // NO SE PINTA NADA AQUI: lo unico que esta funcion sabe es que la orden salio. El sello
  // lo escribe el ACUSE, que es el unico que ha visto el binario. Es el mismo criterio
  // que LEER_RTC de arriba.
  if (btnLeerVersion) {
    btnLeerVersion.addEventListener('click', () => {
      if (!enviarComandoFirmware('VERSION')) return;
      showToast('Consulta de version enviada');
      addEvent('cyan', 'CONSULTA DE VERSION enviada a este poste. No cambia nada: ni una ' +
                       'luz, ni un modo, ni el reloj. Espere el acuse: trae el commit del ' +
                       'que salio el firmware que hay CARGADO, que no tiene por que ser el ' +
                       'ultimo que se compilo.');
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
      // Y NO SE DICE "sincronizado": la orden tiene varios finales y la app solo sabe que
      // salio. La contesta el puente ESP32 del poste conectado (ACK_TEXTO / ERR_MOTIVO),
      // no el STM32 (D-26).
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
      if (!enviarComandoFirmware('TEST_LEDS')) return;
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
      if (targetId === 'tab-depuracion') { renderDepuracion(); renderDiario(); }
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
    // Y lo mismo con el aviso de via, por el motivo de la segunda mitad del comentario
    // de arriba: cerrar es CANCELAR. Una orden que abre paso no se ejecuta porque el
    // operario conteste el dialogo SIGUIENTE, que puede ser de otra maniobra. No se
    // toca `confirmacionVia`: ese es el vale ya concedido y tiene su propia vigencia.
    if (modal === viaModal) {
      state.accionViaPendiente = null;
      state.ordenViaPendiente = null;
    }
  }

  if (btnDevice && btModal) {
    btnDevice.addEventListener('click', () => {
      abrirModalDeEquipos();
    });
  }

  // Los dos atajos de "conectarse al otro poste" abren esta MISMA puerta, y no una copia
  // suya: la eleccion del equipo sigue siendo del tecnico sobre la lista real. La app no
  // sabe que MAC es el otro poste -- y adivinarlo seria volver a los dos data-mac
  // escritos a mano que N-124 tuvo que retirar --, asi que lo que hace el atajo es
  // llevarle a la lista, no elegir por el.
  function abrirModalDeEquipos() {
    if (!btModal) return;
    openModal(btModal);
    // El modal es tambien el sitio desde donde se SUELTA un poste, asi que al abrirlo el
    // boton tiene que decir la verdad de este instante -y nombrar el equipo del que se
    // saldria-. Ver 6.pre.
    actualizarBotonDesconectar();
    // N-124: la lista ya no viene en el HTML, asi que el modal se abriria vacio y haria
    // falta un segundo toque para ver algo. Con radio se rellena al abrir -es la misma
    // llamada que hace el boton-; sin radio esto vuelve en silencio y queda el aviso del
    // HTML diciendo que hay que pulsar Buscar.
    escanearEquipos(false);
  }

  if (btnIrAlEsclavo) {
    btnIrAlEsclavo.addEventListener('click', () => {
      addEvent('cyan', 'Las luces del POSTE 2 ya salen en esta pantalla (el POSTE 1 las ' +
                       'publica). Esto abre su ventana de DIAGNOSTICO, que es lo que no ' +
                       'viaja en la trama. Abriendo la lista de equipos.');
      abrirModalDeEquipos();
    });
  }
  if (btnIrAlMaestro) {
    btnIrAlMaestro.addEventListener('click', () => {
      addEvent('cyan', 'El cruce se opera desde el POSTE 1. Abriendo la lista de equipos ' +
                       'para conectarse a el.');
      abrirModalDeEquipos();
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

  [btModal, siteModal, pinModal, viaModal].forEach(m => {
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
      const accion = state.accionPendiente;
      closeModal(pinModal);
      state.pinVerificado = true;
      if (accion) accion();   // el PIN autoriza la orden; NO sube el rol (D-56: eso es la clave de administracion)
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
      r += `⏱️ *Hora RTC:* ${state.hora || 'sin datos'} | *Pluma:* ${state.pluma || 'sin datos'} | *Cámaras:* ${state.cam || 'sin datos'}\n`;
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
    renderLights();   // la frase del centro la escribe solo renderLights() (SPEC_4 9.9 (c)); las luces, como estaban
    state.countdown = null; state.countdownImposible = false; updateCountdownRing();   // '--' y gris (D-83)
    // El enlace se retira POR EL MISMO CAMINO que lo pinta, con la lectura vacia.
    // Escribir aqui '--' a mano abriria un segundo escritor de esos widgets, y con dos
    // escritores vuelve a poder aparecer uno que pinte algo que no vino en una trama.
    pintarEnlace(ENLACE_SIN_DATO);
    if (batVoltageEl) { batVoltageEl.textContent = '-- V'; batVoltageEl.classList.remove('green'); }
    if (batStatusEl) batStatusEl.textContent = 'Sin datos del equipo';
    // N-150: LA HORA TAMBIEN ENVEJECE, Y PEOR QUE LAS DEMAS. El contador, el RF y la
    // bateria se quedan quietos y quietos ya parecen viejos; un reloj que se quedo en
    // 18:25:03 sigue teniendo cara de hora buena diez minutos despues, y el tecnico lo
    // compara con el suyo y concluye que el equipo va atrasado. Se retira por el mismo
    // camino que la pinta, sin un segundo escritor.
    state.hora = null;
    pintarHoraEquipo();
    // N-153: LA PLUMA ENVEJECE COMO LA HORA, Y PEOR. Un "PLUMA ARRIBA" de hace diez
    // minutos tiene la misma cara que uno de ahora, y quien lo lee cruza por debajo. Se
    // retira el valor y se dice que no hay enlace; pintarPluma() sabe distinguir eso de
    // un equipo que no publica el campo.
    state.pluma = null;
    pintarPluma();
    // D-13: LAS CAMARAS ENVEJECEN COMO LA PLUMA. Un 'OK' de hace diez minutos tiene
    // la misma cara que uno de ahora, y este campo es el unico sitio donde una camara
    // pegada se nota -en fase 1 el vigilante no toca el ciclo-. Se retira por el mismo
    // camino que lo pinta, sin un segundo escritor.
    state.cam = null;
    pintarCamaras();
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
    // La caja de Alcance se retira igual que el badge: state.modo se queda con el
    // ultimo valor conocido (no lo borra esta funcion, ver arriba), y sin este `if`
    // quedaria una caja de ALCANCE encendida sobre un equipo que ya no habla.
    if (alcanceCajaEl) alcanceCajaEl.hidden = true;

    // Sin telemetria no se sabe el modo, asi que el control que depende de el se
    // apaga. Dejarlo habilitado con el ultimo modo conocido seria decidir con un dato
    // vencido, que es lo mismo que se retira dos lineas mas arriba con los numeros.
    actualizarDemanda();

    if (eraConectado) {
      // ESTO ES UNA INFERENCIA, NO UN AVISO DEL SISTEMA, y se dice con esas palabras.
      // Nadie ha notificado nada: lo unico que consta es que no llega telemetria, y de
      // ahi se DEDUCE que el enlace no esta. MEDIDO en las dos puntas: el $STATUS sale
      // cada 2000 ms (Maestro/src/bluetooth.cpp, `ahora - tUltimaTelemetria >= 2000`, y
      // su comentario nombra a este vigilante y su cota de 5000 ms), asi que
      // TIMEOUT_ENLACE_MS son dos periodos y medio. El aviso REAL, cuando el plugin lo
      // manda, entra por el callback de fallo de connect() -- ver 6.pre -- y ese si dice
      // "se ha caido" en vez de deducirlo.
      //
      // Y AQUI SE OFRECE LA SALIDA, que es lo que faltaba. Reportado el 04/09: "no
      // conecta despues de que el maestro se apaga y prende... reinicie la aplicacion y
      // comenzo". La app declaraba la caida correctamente y despues no daba ningun
      // camino de vuelta, asi que el unico que quedaba era cerrarla.
      addEvent('red', 'Enlace perdido: el equipo lleva mas de ' +
                      (TIMEOUT_ENLACE_MS / 1000) + ' s sin emitir telemetria. Es una ' +
                      'DEDUCCION del silencio, no un aviso del sistema. Si el equipo se ' +
                      'reinicio, el socket anterior ya no sirve: pulse el icono de ' +
                      'antena y despues RECONECTAR. No hace falta cerrar la app.');
      // El socket que queda es de una sesion que ya no existe. Se suelta AQUI para que
      // el siguiente intento arranque limpio, y porque mientras siga contando como
      // abierto la guarda de enviarComandoFirmware() deja salir ordenes que no tienen
      // por donde irse -- que es el defecto de N-122 otra vez, ahora por caducidad.
      if (state.connected) desconectarEquipo(null);
      actualizarBotonDesconectar();
      showToast('⚠️ Enlace perdido - se puede reconectar');
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
  renderDiario();
  marcarSinEnlace();
  // Arranca sin punta: los dos mandos de emergencia a la vista, cada uno con su poste
  // escrito. En cuanto se sepa cual hay delante quedara solo el que corresponde.
  actualizarEmergencia();
  setInterval(vigilarEnlace, 1000);
  // La caducidad por inactividad se mira una vez cada 10 s y no cada segundo: el limite
  // son 5 min y la precision que se gana no la nota nadie, mientras que un temporizador
  // menos por segundo si se nota en la bateria de un telefono que pasa el turno con la
  // pantalla encendida al sol.
  setInterval(vigilarAutorizacion, 10000);
  Telefono.instalar(window);   // atras de Android, hojas arriba, hueco de la barra
});
