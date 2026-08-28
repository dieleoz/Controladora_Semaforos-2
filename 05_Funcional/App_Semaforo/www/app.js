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
    rfQuality: null,
    rfRtt: null,
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
  const batVoltageEl = document.getElementById('bat-voltage');
  const batStatusEl = document.getElementById('bat-status');

  // Operario Field Buttons
  const btnOpAuto = document.getElementById('btn-op-auto');
  const btnOpStep = document.getElementById('btn-op-step');
  const btnOpAmber = document.getElementById('btn-op-amber');
  const btnOpEmergency = document.getElementById('btn-op-emergency');

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
  // Las ordenes que el firmware acepta SIN autenticar. Hoy es una sola y esta
  // razonada en bluetooth.cpp:70-82: el rojo de emergencia lo puede dar cualquiera
  // que vea el accidente, porque el PIN guarda lo que ABRE paso, no lo que lo para.
  // Se escribe aqui, con el nombre EXACTO que viaja por el cable, para que el censo
  // del pack app_01_comandos vea lo mismo que ve el micro.
  const SIN_PIN = ['FORZAR_ROJO'];

  function enviarComandoFirmware(comando, args = '') {
    // La excepcion es el rojo de emergencia: bluetooth.cpp:70-82 lo acepta SIN PIN a
    // proposito -el PIN guarda lo que abre paso, no lo que lo para-.
    if (!SIN_PIN.includes(comando) && !state.pinVerificado) {
      console.warn('[TX BLOQUEADO] sin PIN verificado:', comando);
      addEvent('red', 'Comando ' + comando + ' no enviado: falta autorizacion con PIN.');
      return;
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
  // 2. RENDER DE SEMÁFOROS 3D Y CUENTA REGRESIVA
  // =========================================================================
  function renderLights() {
    if (!s1Red || !s1Amber || !s1Green || !s2Red || !s2Amber || !s2Green) return;

    [s1Red, s1Amber, s1Green, s2Red, s2Amber, s2Green].forEach(l => l.classList.remove('active'));

    switch (state.estadoLuces) {
      case 'V1_R2':
        s1Green.classList.add('active');
        s2Red.classList.add('active');
        s1Text.textContent = 'VERDE (PASO)';
        s1Text.style.color = 'var(--green-lamp)';
        s2Text.textContent = 'ROJO (ESPERA)';
        s2Text.style.color = 'var(--red-text)';
        phaseDescEl.textContent = 'FASE: SENTIDO 1 (P1)';
        break;

      case 'Y1_R2':
        s1Amber.classList.add('active');
        s2Red.classList.add('active');
        s1Text.textContent = 'AMARILLO';
        s1Text.style.color = 'var(--amber-lamp)';
        s2Text.textContent = 'ROJO (ESPERA)';
        s2Text.style.color = 'var(--red-text)';
        phaseDescEl.textContent = 'PRECAUCIÓN SENTIDO 1';
        break;

      case 'R1_R2':
      case 'ALL_RED':
        s1Red.classList.add('active');
        s2Red.classList.add('active');
        s1Text.textContent = 'ROJO (DESPEJE)';
        s1Text.style.color = 'var(--red-text)';
        s2Text.textContent = 'ROJO (DESPEJE)';
        s2Text.style.color = 'var(--red-text)';
        phaseDescEl.textContent = 'DESPEJE TOTAL CALZADA';
        break;

      case 'R1_V2':
        s1Red.classList.add('active');
        s2Green.classList.add('active');
        s1Text.textContent = 'ROJO (ESPERA)';
        s1Text.style.color = 'var(--red-text)';
        s2Text.textContent = 'VERDE (PASO)';
        s2Text.style.color = 'var(--green-lamp)';
        phaseDescEl.textContent = 'FASE: SENTIDO 2 (P2)';
        break;

      case 'R1_Y2':
        s1Red.classList.add('active');
        s2Amber.classList.add('active');
        s1Text.textContent = 'ROJO (ESPERA)';
        s1Text.style.color = 'var(--red-text)';
        s2Text.textContent = 'AMARILLO';
        s2Text.style.color = 'var(--amber-lamp)';
        phaseDescEl.textContent = 'PRECAUCIÓN SENTIDO 2';
        break;

      case 'AMBAR_FAIL':
        s1Amber.classList.add('active');
        s2Amber.classList.add('active');
        s1Text.textContent = 'ÁMBAR DESTELLO';
        s1Text.style.color = 'var(--amber-lamp)';
        s2Text.textContent = 'ÁMBAR DESTELLO';
        s2Text.style.color = 'var(--amber-lamp)';
        phaseDescEl.textContent = 'MODO ÁMBAR PRECAUCIÓN';
        break;
    }

    // Badge Modo
    if (badgeModoEl) {
      if (state.modo === 'AUTO') {
        badgeModoEl.className = 'badge badge-auto';
        badgeModoEl.textContent = '🟢 AUTOMÁTICO';
      } else if (state.modo === 'MANUAL') {
        badgeModoEl.className = 'badge badge-auto';
        badgeModoEl.style.background = 'rgba(0,240,255,0.15)';
        badgeModoEl.style.borderColor = 'var(--cyan-neon)';
        badgeModoEl.style.color = 'var(--cyan-neon)';
        badgeModoEl.textContent = '✋ MODO MANUAL';
      } else if (state.modo === 'AMBAR') {
        badgeModoEl.className = 'badge badge-auto';
        badgeModoEl.style.background = 'rgba(255,179,0,0.15)';
        badgeModoEl.style.borderColor = 'var(--amber-lamp)';
        badgeModoEl.style.color = 'var(--amber-lamp)';
        badgeModoEl.textContent = '🟡 ÁMBAR PRECAUCIÓN';
      } else if (state.modo === 'ROJO_TOTAL') {
        badgeModoEl.className = 'badge badge-auto';
        badgeModoEl.style.background = 'rgba(255,30,68,0.2)';
        badgeModoEl.style.borderColor = 'var(--red-lamp)';
        badgeModoEl.style.color = 'var(--red-lamp)';
        badgeModoEl.textContent = '🛑 ROJO TOTAL';
      }
    }
  }

  function updateCountdownRing() {
    if (!cdNumEl || !ringProgressEl) return;
    const current = Math.max(0, state.countdown);
    cdNumEl.textContent = current;

    const total = Math.max(1, state.countdownMax);
    const fraction = current / total;
    const circumference = 251.32;
    const offset = circumference - (fraction * circumference);
    ringProgressEl.style.strokeDashoffset = offset;

    if (state.estadoLuces.includes('V')) {
      ringProgressEl.className = 'ring-fill green';
      cdNumEl.className = 'ring-num green';
    } else if (state.estadoLuces.includes('Y') || state.estadoLuces === 'AMBAR_FAIL') {
      ringProgressEl.className = 'ring-fill amber';
      cdNumEl.className = 'ring-num amber';
    } else {
      ringProgressEl.className = 'ring-fill red';
      cdNumEl.className = 'ring-num red';
    }
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
  if (btnOpAuto) {
    btnOpAuto.addEventListener('click', () => {
      if (!state.pinVerificado) { pedirPin(() => btnOpAuto.click()); return; }
      enviarComandoFirmware('SET_MODO', 'AUTO');
      addEvent('cyan', 'Operario: orden MODO AUTOMATICO enviada al equipo.');
    });
  }

  if (btnOpStep) {
    btnOpStep.addEventListener('click', () => {
      if (!state.pinVerificado) { pedirPin(() => btnOpStep.click()); return; }
      enviarComandoFirmware('MANUAL:CAMBIAR_TURNO');
      addEvent('cyan', 'Operario: orden CAMBIAR TURNO enviada al equipo.');
    });
  }

  if (btnOpAmber) {
    btnOpAmber.addEventListener('click', () => {
      if (!state.pinVerificado) { pedirPin(() => btnOpAmber.click()); return; }
      enviarComandoFirmware('SET_MODO', 'AMBAR');
      addEvent('cyan', 'Operario: orden MODO AMBAR enviada al equipo.');
    });
  }

  if (btnOpEmergency) {
    btnOpEmergency.addEventListener('click', () => {
      // Forma SIN PIN: es la que el firmware espera para la parada de emergencia, y
      // la rama que la construye llevaba desde el rewrite sin un solo llamador.
      enviarComandoFirmware('FORZAR_ROJO');
      addEvent('red', 'ALERTA: orden ROJO TOTAL DE EMERGENCIA enviada al equipo.');
    });
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
    if (!line.startsWith('$')) return;
    const parts = line.split('*')[0].split(',');
    const header = parts[0];

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
      }
      // N-62: aqui se leian SITE y PAIR, y NINGUNA punta los emite. No daba error:
      // dejaba la pantalla igual para siempre, que es la forma silenciosa del fallo.
      // SITE lo gobierna el gestor de cruces de la propia app (LocalStorage) y PAIR
      // depende de FW-PAIR, que todavia no esta en el firmware.
      if (data.SERIE) {
        state.serie = data.SERIE;
      }
      if (data.MODO) {
        state.modo = data.MODO;
      }
      if (data.ESTADO) {
        state.estadoLuces = data.ESTADO;
      }
      if (data.MODO || data.ESTADO) {
        renderLights();
      }
      if (data.T) {
        state.countdown = parseInt(data.T, 10) || 0;
        updateCountdownRing();
      }
      if (data.RF && rfQualityEl) {
        rfQualityEl.textContent = (parseInt(data.RF, 10) || 0) + '%';
      }
      if (data.RTT && rfRttEl) {
        rfRttEl.textContent = data.RTT;
      }
      if (data.BAT && batVoltageEl) {
        // OJO: hoy este numero NO es una medida. Las dos puntas emiten el campo con el
        // literal "BAT:12.6" en el snprintf (Maestro/src/bluetooth.cpp:245 y
        // Esclavo/src/bluetooth.cpp:215): no hay divisor ni ADC detras. La app no puede
        // distinguirlo de una lectura real, asi que lo pinta y lo DICE debajo. El
        // arreglo es del firmware; mientras tanto nadie debe decidir por este valor.
        state.battery = parseFloat(data.BAT);
        batVoltageEl.textContent = Number.isFinite(state.battery)
          ? state.battery.toFixed(1) + 'V' : '-- V';
        if (batStatusEl) batStatusEl.textContent = 'Valor fijo del firmware, no medido';
      }
      if (data.HORA) {
        state.hora = data.HORA;
      }
    } else if (header === '$ALARM') {
      const data = _camposNmea(parts);
      showToast('ALERTA: ' + (data.EVENTO || 'Fallo detectado'));
      addEvent('red', 'Caja Negra: ' + (data.EVENTO || 'FALLO') + ' - ' + (data.CAUSA || '') + ' (Accion: ' + (data.ACCION || '') + ')');
    } else if (header === '$ACK') {
      // El firmware acusa CADA orden que acepta, y la app se lo tragaba: solo pintaba
      // los rechazos. De modo que el operario veia "orden enviada" -que es lo que sabe
      // la app- y nunca "orden aceptada" -que es lo que sabe el equipo-, que son cosas
      // distintas cuando el enlace es una radio. Ahora la unica confirmacion que se
      // muestra viene del equipo.
      const data = _camposNmea(parts);
      const cual = data.CMD || '?';
      addEvent('green', 'Equipo: orden [' + cual + '] ACEPTADA' +
                        (data.RESULT ? ' (' + data.RESULT + ')' : ''));
      showToast('Aceptado por el equipo: ' + cual);
    } else if (header === '$EVENT') {
      // $EVENT es la bitacora del propio equipo -quien movio que y desde donde-. No la
      // leia nadie, asi que el registro de eventos de la app solo contenia lo que la
      // app misma habia hecho: una bitacora que no sabe nada de lo que pasa en el poste.
      const data = _camposNmea(parts);
      addEvent('cyan', 'Equipo [' + (data.ORIGEN || 'FIRMWARE') + ']: ' +
                       (data.DETALLE || '') + (data.HORA ? ' - ' + data.HORA : ''));
    } else if (header === '$ERR') {
      const data = _camposNmea(parts);
      // Un rechazo del firmware NO se oculta: si el equipo dijo que no, la pantalla
      // tiene que decirlo tambien, o el operario se va creyendo que la orden entro.
      addEvent('red', 'Rechazo de Firmware: [' + (data.CMD || '?') + '] ' + (data.DESC || ''));
      showToast('Rechazado por el equipo: ' + (data.DESC || 'ver eventos'));
    }
  }

  // Despachador unico de los botones que declaran su orden en data-cmd. Un solo sitio
  // que traduce el atributo a comando, para que el boton del HTML y lo que sale por
  // Bluetooth no puedan divergir: el pack app_01_comandos lee justo ese atributo.
  //
  // LAS DOS PUNTAS ACEPTAN CONJUNTOS DISTINTOS, y la app mandaba a ciegas. SOLICITAR_PASO
  // solo lo entiende el Esclavo (Esclavo/src/bluetooth.cpp:128) y SET_MODO solo el
  // Maestro (Maestro/src/bluetooth.cpp:123-137): pulsados contra la punta equivocada
  // devolvian $ERR,CMD:DESCONOCIDO y el boton parecia roto. El nodo lo dice $STATUS.
  const SOLO_MAESTRO = ['SET_MODO', 'MANUAL:CAMBIAR_TURNO', 'SET_TIEMPOS', 'TEST_LEDS'];
  const SOLO_ESCLAVO = ['SOLICITAR_PASO'];

  function puntaCorrecta(comando) {
    if (SOLO_MAESTRO.includes(comando) && state.node === 'ESCLAVO') return 'MAESTRO';
    if (SOLO_ESCLAVO.includes(comando) && state.node !== 'ESCLAVO') return 'ESCLAVO';
    return null;
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
        showToast('Esa orden la atiende el ' + punta + ', no esta punta');
        addEvent('red', 'Orden ' + orden + ' no enviada: la acepta el ' + punta +
                        ' y ahora mismo hay un ' + (state.node || '?') + ' al otro lado.');
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
  const btDevices = document.querySelectorAll('.bt-device-item');
  btDevices.forEach(dev => {
    dev.addEventListener('click', () => {
      const name = dev.getAttribute('data-name');
      const node = dev.getAttribute('data-node');
      const mac = dev.getAttribute('data-mac');

      // N-75: el PIN que viajara en CMD:PIN: sale del selector del modal, no de un
      // literal. Si alguien cambia el PIN del equipo, se cambia aqui y el pack
      // documentos_03 comprueba que el elegido sea uno de los que acepta el C++.
      const pinSel = document.querySelector('input[name="bt_pin_opt"]:checked');
      if (pinSel) state.correctPin = pinSel.value;

      state.connected = true;
      state.deviceName = name;
      state.node = node;
      state.deviceMac = mac;

      if (nodeNameEl) nodeNameEl.textContent = node === 'MAESTRO' ? '👑 MAESTRO (POSTE 1)' : '📡 ESCLAVO (POSTE 2)';
      // Abrir el socket NO es tener al equipo hablando: el modulo Bluetooth puede
      // emparejar con el poste alimentado y el micro colgado. Hasta que no llegue el
      // primer $STATUS la cabecera dice "Esperando", y quien lo pone en verde es
      // marcarConEnlace() desde la trama. Antes se pintaba "Enlazado" aqui mismo, con
      // lo cual el rotulo confirmaba el clic del usuario, no la presencia del equipo.
      //
      // Las dos cosas que se pintan aqui dicen COSAS DISTINTAS, y antes decian la misma:
      //   - btnDevice .connected = hay un equipo ELEGIDO y la suscripcion serie abierta.
      //     Eso si es cierto en este instante, y es lo que el boton "Dispositivo" gobierna.
      //   - el punto de estado y el rotulo = el equipo esta HABLANDO. Eso no se sabe
      //     todavia, asi que se queda apagado hasta el primer $STATUS.
      // El modulo Bluetooth empareja igual con un poste alimentado y el micro colgado.
      if (btnDevice) btnDevice.className = 'btn-top btn-device connected';
      if (btStatusDot) btStatusDot.className = 'status-dot';
      if (btBtnText) btBtnText.textContent = 'Esperando equipo';

      // N-75: sin esto la app no oye al equipo. El firmware emite $STATUS solo, cada
      // segundo, asi que NO se pide nada al conectar (N-66: GET_STATUS no existe en
      // ninguna punta y lo primero que veia el tecnico era un $ERR).
      if (typeof window !== 'undefined' && window.bluetoothSerial) {
        window.bluetoothSerial.subscribe(
          '\n',
          (data) => {
            if (data && String(data).trim()) parseNmeaTelemetry(String(data).trim());
          },
          (err) => console.warn('Error en suscripcion serie:', err)
        );
      }

      closeModal(btModal);
      showToast(`🔗 Enlazado a: ${name}`);
      addEvent('green', `Bluetooth conectado con éxito: ${name} (${mac})`);
    });
  });

  if (btnScanBluetoothLive) {
    btnScanBluetoothLive.addEventListener('click', () => {
      // ESTE BOTON NO ESCANEABA NADA. Esperaba un segundo con setTimeout y anunciaba
      // "2 Modulos Semaforicos encontrados", que es justo el numero de filas fijas que
      // trae el HTML. Sin radio, sin permisos y sin equipo delante decia lo mismo. Un
      // hallazgo que no salio de una busqueda es un dato inventado con forma de medida.
      if (typeof window === 'undefined' || !window.bluetoothSerial ||
          typeof window.bluetoothSerial.list !== 'function') {
        showToast('Escaneo no disponible fuera del APK');
        addEvent('red', 'Escaneo Bluetooth no realizado: no hay radio disponible en ' +
                        'este entorno. La lista de abajo son los equipos conocidos, no ' +
                        'un resultado de busqueda.');
        return;
      }
      // El rotulo NO nombra el modulo. En obra ya se ha cambiado de radio mas de una
      // vez -y ahora se habla de poner un ESP32 al lado-, asi que un texto que diga
      // "HC-05" queda desmintiendo al equipo el dia que se sustituya. "El enlace" vale
      // para todos y no hay que rectificarlo.
      showToast('🔍 Buscando equipos...');
      window.bluetoothSerial.list(
        (dispositivos) => {
          const n = Array.isArray(dispositivos) ? dispositivos.length : 0;
          showToast(n + ' equipo(s) emparejado(s)');
          addEvent(n ? 'green' : 'red', 'Escaneo Bluetooth: ' + n + ' equipo(s).');
        },
        (err) => {
          // No se finge un resultado vacio: no encontrar y no poder buscar son cosas
          // distintas, y solo una de las dos permite concluir que no hay equipo.
          showToast('El escaneo fallo');
          addEvent('red', 'Escaneo Bluetooth fallido: ' + err + '. No se sabe que hay.');
        }
      );
    });
  }

  // =========================================================================
  // 7. FORMULARIO DE TIEMPOS DE CICLO (GUARDAR Y APLICAR)
  // =========================================================================
  if (formTiempos) {
    formTiempos.addEventListener('submit', (e) => {
      e.preventDefault();
      const verde = parseInt(numTiempoVerde.value, 10);
      const rojo = parseInt(numTiempoRojo.value, 10);
      const despeje = parseInt(numTiempoDespeje.value, 10);

      if (verde < 1 || verde > 15 || rojo < 1 || rojo > 15 || despeje < 10 || despeje > 90) {
        showToast('❌ Error: Tiempos fuera de rango permitido.');
        return;
      }

      state.tiempoVerdeMin = verde;
      state.tiempoRojoMin = rojo;
      state.tiempoDespejeSeg = despeje;
      state.countdown = verde * 60;
      state.countdownMax = verde * 60;

      enviarComandoFirmware('SET_TIEMPOS', `${verde},${rojo},${despeje}`);
      updateCountdownRing();
      showToast(`💾 Tiempos guardados: Verde ${verde}m · Rojo ${rojo}m · Despeje ${despeje}s`);
      addEvent('cyan', `Ajustes aplicados: Verde=${verde}min, Rojo=${rojo}min, Despeje=${despeje}seg.`);
    });
  }

  // =========================================================================
  // 8. ASISTENTE COURIER RTC & SINCRONIZACIÓN
  // =========================================================================
  if (btnCourierCapture) {
    btnCourierCapture.addEventListener('click', () => {
      const now = new Date().toLocaleTimeString();
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

      const comp = CourierRTC.calcularCompensacion(state.courierSnapshot, Date.now());
      const today = new Date().toISOString().slice(0, 10);
      enviarComandoFirmware('SET_RTC', `${today},${comp.horaCompensada}`);
      showToast(`🚀 Sincronizado en Esclavo. Hora compensada: ${comp.horaCompensada}`);
      addEvent('green', `Courier RTC: Inyección en Esclavo exitosa (Viaje: ${comp.elapsedSeg}s).`);

      if (btnCourierInject) btnCourierInject.disabled = true;
    });
  }

  if (btnSyncRtc) {
    btnSyncRtc.addEventListener('click', () => {
      const now = new Date().toLocaleTimeString();
      const today = new Date().toISOString().slice(0, 10);
      enviarComandoFirmware('SET_RTC', `${today},${now}`);
      showToast(`⏱️ Reloj RTC DS3231 sincronizado a las ${now}`);
      addEvent('green', `Reloj del Semáforo ajustado con el celular: ${now}.`);
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
    });
  });

  // =========================================================================
  // 10. MODALES Y TECLADO PIN
  // =========================================================================
  function openModal(modal) {
    if (modal) modal.classList.add('active');
  }

  function closeModal(modal) {
    if (modal) modal.classList.remove('active');
  }

  if (btnDevice && btModal) {
    btnDevice.addEventListener('click', () => openModal(btModal));
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

  [btModal, siteModal, pinModal].forEach(m => {
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

  const pinButtons = document.querySelectorAll('.pin-btn[data-key]');
  pinButtons.forEach(btn => {
    btn.addEventListener('click', () => {
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
      state.pin = state.pin.slice(0, -1);
      updatePinDisplay();
    });
  }

  if (btnPinOk) {
    btnPinOk.addEventListener('click', () => {
      if (state.pin.length === 4) validatePin();
    });
  }

  function validatePin() {
    if (state.pin === state.correctPin) {
      closeModal(pinModal);
      state.pinVerificado = true;
      // Dos usos distintos: autorizar un comando pendiente NO asciende a TECNICO.
      // El operario que da paso desde el suelo no queda con el menu de ajustes abierto.
      const accion = state.accionPendiente;
      state.accionPendiente = null;
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
      r += `👑 *Nodo:* ${state.node} | Modo: ${state.modo}\n`;
      r += `🔋 *Batería:* ${state.battery}V | Enlace RF: ${state.rfQuality}%\n\n`;
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
    state.telemetriaViva = true;
    if (btStatusDot) btStatusDot.className = 'status-dot connected';
    if (btBtnText) btBtnText.textContent = 'Enlazado';
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
    if (rfQualityEl) rfQualityEl.textContent = '--';
    if (rfRttEl) rfRttEl.textContent = 'Sin datos del equipo';
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

    if (eraConectado) {
      addEvent('red', 'Enlace perdido: el equipo lleva mas de ' +
                      (TIMEOUT_ENLACE_MS / 1000) + ' s sin emitir telemetria.');
    }
  }

  function vigilarEnlace() {
    if (!state.ultimoStatusMs) { marcarSinEnlace(); return; }
    if (Date.now() - state.ultimoStatusMs > TIMEOUT_ENLACE_MS) marcarSinEnlace();
  }


  setInterval(() => {
    const now = new Date();
    if (rtcSyncDigits) rtcSyncDigits.textContent = now.toTimeString().split(' ')[0];

    // Si el servidor puente Python está activo, toma la telemetría del simulador C++
    if (typeof fetch !== 'undefined') {
      fetch('/api/status_json')
        .then(r => r.json())
        .then(data => {
          state.modo = data.modo;
          state.estadoLuces = data.estado;
          state.countdown = data.countdown;
          state.countdownMax = data.countdown_max;
          state.battery = data.bat;
          state.rfQuality = data.rf;
          state.rfRtt = data.rtt;
          if (batVoltageEl) batVoltageEl.textContent = `${data.bat} V`;
          if (rfQualityEl) rfQualityEl.textContent = `${data.rf}%`;
          if (rfRttEl) rfRttEl.textContent = `RTT: ${data.rtt} ms`;
          state.ultimoStatusMs = Date.now();
          marcarConEnlace();
          renderLights();
          updateCountdownRing();
        })
        .catch(() => {
          // El puente no contesta. No se declara caido el enlace aqui: en el APK no
          // hay puente y el dato bueno llega por Bluetooth. Decide el watchdog, que
          // mira la unica senal que sirve en las dos vias -cuando hablo el equipo-.
          vigilarEnlace();
        });
    } else {
      vigilarEnlace();
    }
  }, 1000);

  // Inicialización: el tablero nace declarando que no tiene datos, no fingiendo un
  // cruce en marcha. En cuanto llegue el primer $STATUS lo pintara la telemetria.
  renderEvents();
  marcarSinEnlace();
  setInterval(vigilarEnlace, 1000);
});
