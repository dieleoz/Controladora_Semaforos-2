// ===== tests/dom_usabilidad.js =====
// Pruebas jsdom de APP-1 (OPTIMIZACIONES.md) y de la cinta sin enlace (SPEC_4, apartado 4). No
// corre sola: la llama test_dom_execution.js con SU montarAppLimpia y SU assert, asi que
// cuentan en el mismo RESULTADO JSDOM que lee la compuerta. Cada bloque dice de donde
// sale el valor que asevera; lo que es solo aspecto (colores, alturas en pixeles) no se
// mide aqui: jsdom no pinta. Eso se mide con herramientas_medir_*.js en un Chrome real.
// D-80 (SPEC_4 9.9 (a)): la jerga, UNA expresion, copiada en la spec. La usan tambien
// test_unitarios_app.js y test_dom_execution.js (toasts de D-101) por require.
const JERGA = new RegExp(String.raw`\bRSSI\b|\bRTT\b|\bMAC\b|\bNODE\b|\$STATUS|\$EVENT|\$ACK|\$ERR|\bCMD:|\bD-\d{1,3}\b|\bJ1[4-7]\b|OTRO_PERRO|_MUDO|` +
  String.raw`SET_MODO|SET_DEG_AUTO|SET_RTC|no viaja en esta trama|ESP32|STM32|\b(RF|BAT|RTT|T):--`);
// Zona del operario: body sin .admin-only, sin Eventos, sin script ni style. Devuelve el trozo que casa.
const jergaEnOperario = (d) => {
  const c = d.body.cloneNode(true);
  c.querySelectorAll('.admin-only, #tab-eventos, script, style').forEach(n => n.remove());
  const txt = c.textContent.replace(/\s+/g, ' ');
  const m = JERGA.exec(txt);
  return m ? txt.slice(Math.max(0, m.index - 50), m.index + 30) : null;
};
module.exports = function pruebaUsabilidad(montarAppLimpia, assert) {
  const eventos = (a) => Array.from(a.d.querySelectorAll('.event-item'))
                              .map(n => n.textContent.replace(/\s+/g, ' ')).join(' || ');

  // --- I. SPEC_4 apartado 4: "Una orden que no salio se anota en el Diario y no en la cinta" ---
  // Sin enlace Bluetooth abierto (state.connected falso) enviarComandoFirmware() no llama a
  // bluetoothSerial.write(): por el cable no pasa un byte. Se pulsa ROJO TOTAL, que va sin
  // PIN, para que la unica barrera en juego sea el enlace y no la clave.
  const sin = montarAppLimpia();
  sin.d.getElementById('btn-bt-disconnect').click();
  sin.w.RegistroCrudo.limpiar();
  sin.w.DiarioOrdenes.limpiar();
  sin.tramas.length = 0;
  sin.d.getElementById('btn-op-emergency').click();
  assert(sin.tramas.length === 0,
    `I: sin enlace ROJO TOTAL no escribe un byte (${sin.tramas.join(' | ')})`);
  const enviadas = sin.w.RegistroCrudo.todas().filter(x => x.veredicto === sin.w.RegistroCrudo.ENVIADA);
  assert(enviadas.length === 0,
    `I: y la cinta NO la anota como ENVIADA: por el cable no paso nada (${enviadas.length})`);
  const ords = sin.w.DiarioOrdenes.todas().filter(x => x.clase === 'ORDEN');
  assert(ords.length === 1 && ords[0].salio === false && /enlace/i.test(ords[0].motivoNoSalio || ''),
    `I: el Diario la anota como NO salida y dice por que (${ords.length ? ords[0].motivoNoSalio : '-'})`);
  assert(!/ROJO TOTAL DE EMERGENCIA enviada/.test(eventos(sin)),
    'I: y la bitacora no dice "enviada" de una orden que no salio');
  assert(/FORZAR_ROJO no enviad/.test(eventos(sin)),
    `I: la bitacora dice en rojo que no salio: "${eventos(sin).slice(0, 140)}"`);

  // La otra respuesta (CLAUDE.md 6.2): con enlace abierto la misma pulsacion SI se anota.
  const con = montarAppLimpia();
  con.w.RegistroCrudo.limpiar();
  con.tramas.length = 0;
  con.d.getElementById('btn-op-emergency').click();
  assert(con.tramas.length === 1 &&
         con.w.RegistroCrudo.todas().filter(x => x.veredicto === con.w.RegistroCrudo.ENVIADA).length === 1,
    'I: con enlace abierto la misma orden sale y la cinta la anota ENVIADA');

  // --- A. APP-1.1: SEM-* arriba, el resto plegado -----------------------------------
  // El prefijo se lee del firmware (ROTULO_PREFIJO en contrato.h), no se copia aqui.
  const fs = require('fs');
  const path = require('path');
  const contrato = fs.readFileSync(path.join(__dirname, '..', '..', '..', '01_Firmware',
    'ESP32_Expansion', 'include', 'contrato.h'), 'utf8');
  const prefijo = (/#define\s+ROTULO_PREFIJO\s+"([^"]+)"/.exec(contrato) || [])[1];
  assert(!!prefijo, `A: ROTULO_PREFIJO leido de contrato.h (${prefijo})`);
  const a = montarAppLimpia();
  const LE = a.w.ListaEquipos;
  const caja = a.d.createElement('div');
  const vistos = new Map();
  const eq = (nombre, mac, emp) => vistos.set(mac, { nombre, mac, emparejado: emp });
  eq('Auriculares', '11:11', true);
  eq('22:22', '22:22', false);                      // MAC anonima: pintarEquipos() pone el MAC
  eq(prefijo + 'A1-E', '33:33', false);
  eq('Coche', '44:44', true);
  eq(prefijo + 'A1-M', '55:55', true);
  const n = LE.repintarEquipos(caja, vistos, null);
  const hijos = Array.from(caja.children);
  const plegado = caja.querySelector('details.bt-otros');
  assert(n === 5 && hijos[0].getAttribute('data-name') === prefijo + 'A1-M' &&
         hijos[1].getAttribute('data-name') === prefijo + 'A1-E' && hijos[2] === plegado,
    `A: los ${prefijo}* van primero y a la vista, el resto detras (${hijos.map(h => h.getAttribute('data-name') || h.tagName).join(', ')})`);
  const dentro = plegado ? Array.from(plegado.querySelectorAll('.bt-device-item')) : [];
  assert(!!plegado && !plegado.open && /Otros dispositivos \(3\)/.test(plegado.textContent) &&
         dentro.length === 3 && dentro[2].getAttribute('data-mac') === '22:22',
    'A: el resto va PLEGADO bajo "Otros dispositivos (3)", con la MAC anonima al final');
  assert(/Dirección 55:55/.test(caja.textContent) && !JERGA.test(caja.textContent),   // D-80, SPEC_4 9.9 (a) 7
    `A: la hoja de conexion dice «Dirección», sin jerga: "${(JERGA.exec(caja.textContent) || [''])[0]}"`);
  const solo = new Map([['66:66', { nombre: 'JDY-31', mac: '66:66', emparejado: true }]]);
  const caja2 = a.d.createElement('div');
  LE.repintarEquipos(caja2, solo, null);
  const p2 = caja2.querySelector('details.bt-otros');
  assert(!!p2 && p2.open === true,
    `A: sin ningun ${prefijo}* a la vista el plegado sale ABIERTO: no se esconde el unico candidato`);

  // --- C. APP-1.4: bateria en gris sin dato, verde solo con dato ---------------------
  // SPEC_4 apartado 6: el equipo manda BAT:-- (no la mide). El gris lo pone style.css a lo que
  // no lleva .green; aqui se mide la clase, que es lo que jsdom puede ver.
  const bat = a.d.getElementById('bat-voltage');
  assert(bat.textContent.trim() === '12.9V' && bat.classList.contains('green'),
    `C: con BAT medida (montarAppLimpia manda BAT:12.9) el valor va en verde (${bat.textContent})`);
  const carga = 'STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:30,RF:97,RTT:70,BAT:--,HORA:14:31:01';
  let x = 0; for (const c of carga) x ^= c.charCodeAt(0);
  a.w._btSubscribeCb(`$${carga}*${x.toString(16).toUpperCase().padStart(2, '0')}\n`);
  assert(bat.textContent.trim() === '-- V' && !bat.classList.contains('green'),
    `C: con BAT:-- pinta "-- V" SIN verde (${bat.textContent} / ${bat.className})`);
  const fresca = montarAppLimpia();
  fresca.d.getElementById('btn-bt-disconnect').click();
  // El vigilante del enlace (marcarSinEnlace) repone "-- V" al pasar TIMEOUT_ENLACE_MS sin
  // $STATUS: se adelanta el reloj un minuto y se le deja correr una vuelta.
  const mudo = montarAppLimpia();
  const ahora = mudo.w.Date.now();
  mudo.w.Date.now = () => ahora + 60000;

  // --- D. APP-1.5: AMBAR y AMBAR EMERGENCIA no llevan el mismo icono ------------------
  const icono = (id) => (a.d.querySelector('#' + id + ' .pad-icon') || {}).textContent;
  assert(icono('btn-op-amber') && icono('btn-op-ambar-emergencia') &&
         icono('btn-op-amber') !== icono('btn-op-ambar-emergencia'),
    `D: iconos distintos: ${icono('btn-op-amber')} / ${icono('btn-op-ambar-emergencia')}`);

  // --- F. APP-1.7: sin punto colgante y "NO SE SABE" con sujeto ---------------------
  const rotulos = Array.from(a.d.querySelectorAll('.semaforo-label')).map(r => r.textContent);
  assert(rotulos.length === 2 && rotulos.every(r => !/\u00b7/.test(r)),
    `F: los rotulos de los postes ya no llevan el punto que colgaba (${rotulos.join(' | ')})`);
  const padPoste = fresca.d.getElementById('pad-poste').textContent.trim();
  // Texto decidido por el responsable el 28/09: "algo que se entienda" sin explicacion.
  assert(padPoste === 'Poste sin identificar',
    `F: sin punta la botonera dice "Poste sin identificar" (${padPoste})`);

  // --- E. APP-1.6: la hoja de conexion abre arriba y avisa del permiso ---------------
  const e = montarAppLimpia();
  const hoja = e.d.getElementById('bt-modal');
  e.d.getElementById('modal-bt-close').click();
  hoja.scrollTop = 300;
  e.d.getElementById('btnDevice').click();
  return new Promise(r => setTimeout(r, 1100)).then(() => {
    const bat2 = mudo.d.getElementById('bat-voltage');
    assert(bat2.textContent.trim() === '-- V' && !bat2.classList.contains('green'),
      `C: con el enlace caido vuelve a "-- V" sin verde (${bat2.textContent} / ${bat2.className})`);
    // D-83 (SPEC_4 9.9 (b)): gris es "sin dato"; un valor con "--" no lleva clase ni color de accion.
    const COLOR = /\b(red|amber|green|cyan)\b|-(red|amber|green|cyan)-/;
    ['rf-quality', 'cam-estado', 'cd-num', 'ring-progress', 'bat-voltage'].forEach(id => {
      const el = mudo.d.getElementById(id);
      const cl = el.getAttribute('class') || '', co = el.style.color || '';
      assert((id === 'ring-progress' || /--/.test(el.textContent)) && !COLOR.test(cl) && !COLOR.test(co),
          `C (D-83): #${id} con el enlace caido ("${el.textContent.trim()}") va sin color: class "${cl}", color "${co}"`);
    });
    assert(hoja.classList.contains('active') && hoja.scrollTop === 0,
      `E: al abrir la hoja de conexion vuelve arriba (scrollTop ${hoja.scrollTop})`);
    const aviso = e.d.getElementById('bt-aviso-permiso');
    const lista = e.d.getElementById('bt-device-list-container');
    assert(!!aviso && /Ubicaci/.test(aviso.textContent) &&
           (aviso.compareDocumentPosition(lista) & e.w.Node.DOCUMENT_POSITION_FOLLOWING),
      'E: la linea del permiso de ubicacion esta ANTES de la lista que dispara la busqueda');

    // --- H. atras de Android: cierra la ventana; sin ventana, lo de siempre --------
    const T = e.w.Telefono;
    const llam = [];
    const app = { minimizeApp: () => llam.push('min'), exitApp: () => llam.push('exit'),
                  addListener: (ev, cb) => { llam.push(ev); app.cb = cb; } };
    e.w.Capacitor = { Plugins: { App: app } };
    assert(T.instalarAtras(e.w) === true && llam[0] === 'backButton',
      'H: con @capacitor/app se escucha backButton');
    app.cb({ canGoBack: false });
    assert(!hoja.classList.contains('active') && llam.indexOf('min') < 0,
      'H: con la hoja de conexion abierta, atras la CIERRA y no saca la app');
    const via = e.d.getElementById('pin-modal');
    via.classList.add('active');
    assert(T.alPulsarAtras(e.d, app) === 'cerrada' && !via.classList.contains('active'),
      'H: igual con el teclado del PIN');
    // Sin ventana: doble atras para salir, plazo 2 s (decision del responsable, 28/09).
    const cuenta = (v) => llam.filter(x => x === v).length;
    const t0 = e.w.Date.now();
    let reloj = t0;
    e.w.Date.now = () => reloj;
    const toast = e.d.getElementById('toast-msg');
    assert(T.alPulsarAtras(e.d, app) === 'aviso' && cuenta('exit') === 0 && cuenta('min') === 0 &&
           toast.textContent === 'Pulse otra vez para salir' && toast.classList.contains('show'),
      `H: sin ventana, el primer atras NO sale y avisa "Pulse otra vez para salir" (${toast.textContent})`);
    reloj = t0 + 1999;
    assert(T.alPulsarAtras(e.d, app) === 'salida' && cuenta('exit') === 1 && cuenta('min') === 0,
      'H: un segundo atras dentro de 2 s sale de la app (exitApp)');
    reloj = t0 + 10000;
    assert(T.alPulsarAtras(e.d, app) === 'aviso' && cuenta('exit') === 1,
      'H: pasado el plazo vuelve a pedirlo: el tercer atras solo avisa');
    reloj = t0 + 12001;
    assert(T.alPulsarAtras(e.d, app) === 'aviso' && cuenta('exit') === 1,
      'H: un atras a 2001 ms del aviso tampoco sale: el plazo es de 2 s');

    // --- J. D-80 (SPEC_4 9.9 (a)): la jerga solo en la zona tecnica, en seis fotos -----
    const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                         return c.toString(16).toUpperCase().padStart(2, '0'); };
    const j = montarAppLimpia();
    if (j.w.DegAuto && j.w.DegAuto._timerConsulta) j.w.clearTimeout(j.w.DegAuto._timerConsulta);
    const entra = (carga) => j.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
    const foto = (n) => { const m = jergaEnOperario(j.d); assert(!m, `J (D-80) foto ${n}: jerga en la zona del operario: "${m}"`); };
    j.d.getElementById('btn-bt-disconnect').click();
    j.d.getElementById('btnDevice').click();
    j.d.querySelector('.bt-device-item').click();
    assert(/esperando su primer dato/.test(j.d.getElementById('phase-desc').textContent),
      `J (1): tras conectar y antes del primer dato: "${j.d.getElementById('phase-desc').textContent}"`);
    foto('1 (conectado, sin dato)');
    entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:9,RF:97,RTT:70,BAT:12.9,HORA:14:31:00,ESC:VERDE,PLUMA:ABAJO');
    foto('2 (Maestro AUTO)');
    entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:FALLO COM,T:9,RF:0,RTT:--,BAT:12.9,HORA:14:31:05,ESC:?,PLUMA:ARRIBA');
    foto('3 (FALLO COM, RF:0, ESC:?)');
    entra('STATUS,NODE:ESCLAVO,SERIE:SEM-E-01,MODO:SUBORDINADO,ESTADO:ROJO,T:--,RF:--,RTT:--,BAT:--,HORA:14:31:10');
    entra('EVENT,NODE:ESCLAVO,ORIGEN:ENLACE_RF,DETALLE:RX:100 OK:50 RUIDO:2,HORA:14:31:30');
    foto('4 (Esclavo, ventana del Poste 2)');
    entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:9,RF:97,RTT:70,BAT:12.9,HORA:14:31:40,ESC:VERDE,PLUMA:ABAJO');
    entra('ALARM,NODE:MAESTRO,EVENTO:HORA_ESP32,CAUSA:J17_MUDO,HORA:14:31:41');
    assert(j.d.getElementById('toast-msg').classList.contains('show'), 'J (5): el $ALARM saca el toast');
    foto('5 (alarma J17_MUDO y su toast)');
    entra('ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR');   // 9.9 (a) 5: el «revise J17» de los acuses
    const tAck = j.d.getElementById('toast-msg').textContent;
    assert(/controladora/.test(tAck) && !JERGA.test(tAck) && /avise al tecnico/.test(tAck), `J (5): toast del acuse sin jerga: "${tAck}"`);
    entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:DEGRADADO,ESTADO:ROJO,T:9,RF:0,RTT:0,BAT:12.9,HORA:14:31:50');
    j.d.querySelector('[data-cmd="SET_MODO:MANUAL"]').click();
    ['1', '2', '3', '4'].forEach(k => j.d.querySelector(`.pin-btn[data-key="${k}"]`).click());
    assert(j.d.getElementById('aviso-deg-modal').classList.contains('active'), 'J (6): en DEGRADADO el aviso de paleteros abierto');
    foto('6 (aviso de paleteros)');
  });
};
module.exports.JERGA = JERGA;
