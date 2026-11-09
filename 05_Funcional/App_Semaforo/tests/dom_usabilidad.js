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
  // N-181 U (D-141): «Poste sin identificar» de la botonera y MAESTRO/ESCLAVO bajo cada poste salen: lo dice la cabecera.
  assert(!fresca.d.querySelector('#pad-poste, .pad-header, .semaforo-rol'), 'F: sin rotulo de punta repetido en Trafico (D-141)');

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
    return tareas(montarAppLimpia, assert);
  });
};
// --- K y T: N-181 U (SPEC_4 9.9 (f)-(i); D-141..D-145). Se pulsa por el rotulo VISIBLE, como el tecnico. ---
// Visible: sin hidden ni display:none en la cadena, en la pestana y la ventana activas, fuera de un <details> cerrado
// (salvo su <summary>) y, sin clave, fuera de .solo-admin. jsdom no carga style.css; scroll y 48 px los mide P (CDP).
const oculto = (n) => {
  const admin = n.ownerDocument.body.classList.contains('admin');
  for (let e = n; e && e.nodeType === 1; e = e.parentElement) {
    if (e.hidden || e.style.display === 'none' || (!admin && e.classList.contains('solo-admin'))) return true;
    if (/\b(tab-content|modal-overlay)\b/.test(e.className) && !e.classList.contains('active')) return true;
    const p = e.parentElement;
    if (p && p.tagName === 'DETAILS' && !p.open && e.tagName !== 'SUMMARY') return true;
  }
  return false;
};
const textoVisible = (raiz) => { const w = raiz.ownerDocument.createTreeWalker(raiz, 4); let t = '';
  for (let x; (x = w.nextNode());) if (!oculto(x.parentElement)) t += ' ' + x.textContent;
  return t.replace(/\s+/g, ' ').trim(); };
// D-142 (9.9 (g) 1-2): botones, <summary> y palabras visibles de Trafico.
const cifras = (d) => { const r = d.getElementById('tab-estado'), vis = (s) => Array.from(r.querySelectorAll(s)).filter(n => !oculto(n));
  return { botones: vis('button').length, summary: vis('summary').length,
           palabras: (textoVisible(r).match(/[A-Za-z0-9%\u00C0-\u017F]+/g) || []).length }; };
async function tareas(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0); return c.toString(16).toUpperCase().padStart(2, '0'); };
  const micro = () => new Promise(r => setTimeout(r, 0));
  const AVISOS = ['aviso-camara-pluma', 'aviso-corte', 'carteles-equipo', 'degauto-sinradio', 'aviso-pareja'];
  const app = () => { const a = montarAppLimpia(); a.w.clearTimeout(a.w.DegAuto._timerConsulta);
    a.entra = (c) => a.w._btSubscribeCb(`$${c}*${xor(c)}\n`);
    a.t = (id) => (a.d.getElementById(id) || { textContent: '' }).textContent.replace(/\s+/g, ' ').trim();
    a.ve = (id) => !!a.d.getElementById(id) && !oculto(a.d.getElementById(id));
    a.pin = () => ['1', '2', '3', '4'].forEach(k => a.d.querySelector(`.pin-btn[data-key="${k}"]`).click());
    a.pulsa = (rotulo) => { const n = Array.from(a.d.querySelectorAll('button, summary, .bt-device-item'))
      .find(x => !oculto(x) && x.textContent.includes(rotulo));
      assert(!!n, `T: «${rotulo}» a la vista para pulsarlo`); if (n) n.click(); };
    a.clave = () => { a.pulsa('Operario'); a.d.getElementById('clave-entrada').value = '1234'; a.pulsa('Entrar'); };
    return a; };
  const M = 'STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:45,RF:97,RTT:70,BAT:12.9,HORA:14:31:00,ESC:VERDE,PLUMA:ABAJO,CAM:OK';
  const E = 'STATUS,NODE:ESCLAVO,SERIE:SEM-E-01,MODO:SUBORDINADO,ESTADO:VERDE,T:--,RF:--,RTT:--,BAT:--,HORA:14:31:00,PLUMA:ARRIBA,CAM:OK';

  // K (D-142, 9.9 (g) 1-2): Trafico sin clave y con todo plegado, en las tres fotos de U-S.
  for (const [n, st] of [['sin enlace', null], ['Maestro', M], ['Esclavo', E]]) {
    const a = app(); if (st) a.entra(st); else a.d.getElementById('btn-bt-disconnect').click();
    await micro();
    const c = cifras(a.d);
    assert(c.botones <= 6 && c.summary <= 2 && c.palabras <= 80,
      `K (D-142) ${n}: ${c.botones} botones (<= 6), ${c.summary} summary (<= 2), ${c.palabras} palabras (<= 80)`);
  }
  // K (D-141, D-143, D-145; 9.9 (f), (g) 6, (h)): lo plegado, lo que sale, los rotulos y un nombre con su frase.
  {
    const a = app(), rol = a.d.getElementById('btn-toggle-role');
    const r = () => a.t('role-label') + '|' + rol.title + '|' + rol.getAttribute('aria-label');
    assert(/^Operario\|Operario: manda el cruce/.test(r()) && /Operario/.test(rol.getAttribute('aria-label') || ''), `K (D-145): pastilla Operario: ${r()}`);
    assert(!a.ve('btn-start-test-leds') && !a.ve('btn-ir-al-esclavo'), 'K (D-141): Prueba de focos e ir al POSTE 2, plegados en «Más»');
    a.pulsa('Más');
    assert(a.ve('btn-start-test-leds') && a.ve('btn-ir-al-esclavo'), 'K (D-141): «Más» los abre, sin clave');
    a.pulsa('Eventos');
    const exp = Array.from(a.d.querySelectorAll('#tab-eventos button')).filter(b => !oculto(b));
    assert(exp.length === 1 && /Exportar registro/.test(exp[0].textContent) && !a.d.getElementById('btn-share-whatsapp') &&
           !a.d.getElementById('btn-export-csv'), `K (D-143): Eventos, un solo boton: ${exp.map(b => b.textContent.trim()).join(' | ')}`);
    a.clave();
    assert(/^Técnico\|Técnico: además ajusta/.test(r()) && a.t('tab-btn-diag') === '⚙️ Ajustes', `K (D-145): pastilla Técnico y pestaña Ajustes: ${r()} / ${a.t('tab-btn-diag')}`);
    const H = [['Verde (min)', 'Minutos que dura el verde, de 3 a 15.'], ['Rojo (min)', 'Minutos que dura el rojo, de 3 a 15.'],
      ['Todo rojo entre sentidos (s)', 'Segundos con los dos en rojo para vaciar el tramo, de 10 a 90.'],
      ['Distancia entre postes (m)', 'Con ella la app propone el todo rojo de la noche.'],
      ['Todo rojo al entrar y salir de noche (s)', 'No baja de 180 ni de lo que pide la distancia.'],
      ['Franja 4 de noche', 'Días, horas y color en que destellan los dos postes.'],
      ['Cámaras: veto de la barrera', 'Fuera, la cámara deja de impedir que baje la barrera.'],
      ['Barrera en servicio', 'Fuera, la barrera queda arriba también en rojo (brazo roto).'],
      ['Si se cae la radio', 'Qué hacen los dos postes tras 5 min sin oírse.']];
    const conFrase = (nom, fr) => Array.from(a.d.querySelectorAll('label, strong, h3, summary')).some(e => e.textContent.trim().startsWith(nom) &&
      Array.from(e.parentElement.querySelectorAll('small')).some(s => s.textContent.trim() === fr));
    const faltan = H.filter(([nom, fr]) => !conFrase(nom, fr)).map(x => x[0]);
    assert(!faltan.length, `K (D-145, 9.9 (h)): cada ajuste con su nombre y su frase; faltan: ${faltan.join(' | ')}`);
    a.pulsa('Técnico'); a.pulsa('Operario');   // volver a pedir la clave: su ventana dice que hace
    assert(a.t('clave-titulo') === 'Clave de técnico' && a.t('clave-texto') === 'Enseña los ajustes; cada orden sigue pidiendo el PIN del equipo.',
      `K (9.9 (h)): la ventana de la clave: ${a.t('clave-titulo')} / ${a.t('clave-texto')}`);
    a.pulsa('Cancelar'); a.clave(); a.pulsa('Ajustes');
    assert(!a.ve('btn-demanda') && !a.ve('btn-courier-capture') && !a.ve('btn-leer-rtc') && a.ve('btn-sync-rtc') &&
           !a.ve('btn-leer-version') && a.ve('btn-practica'), 'K (9.9 (f)): Pedir paso fuera de Inteligente; Courier y consultas en «Soporte»');
    a.entra(M.replace('MODO:AUTO', 'MODO:INTELIGENTE'));
    assert(a.ve('btn-demanda'), 'K (9.9 (f)): con MODO:INTELIGENTE, Pedir paso a la vista');
  }

  // T2. Leer el estado del cruce sin pulsar nada.
  const a2 = app(); a2.entra(M); await micro();
  const luz = (id) => a2.d.getElementById(id).classList.contains('active');
  assert(luz('s1-red') && luz('s2-green') && a2.t('cd-num') === '45' && /ABAJO/.test(a2.t('pluma-estado')) &&
         a2.t('resumen-equipo') === 'Radio entre postes: BUENA · Batería 12.9 V · Cámaras bien',
    `T2: POSTE 1 rojo, POSTE 2 verde, cuenta, pluma y resumen: "${a2.t('resumen-equipo')}"`);
  a2.entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:FALLO COM,T:9,RF:0,RTT:--,BAT:12.9,HORA:14:31:05,ESC:?,PLUMA:ARRIBA,CAM:OK');
  await micro();
  assert(/ámbar y pluma arriba: pasar con precaución/.test(a2.t('phase-desc')) && a2.d.querySelector('#resumen-equipo .res-rojo') &&
         a2.ve('resumen-equipo'), `T2: FALLO COM, frase de (c) y la radio del resumen en rojo: "${a2.t('phase-desc')}"`);

  // T1. Conectar: Dispositivo -> Buscar -> la fila. Variante sin emparejar: «Emparejar y conectar» ahi mismo.
  const a1 = app(); a1.d.getElementById('btn-bt-disconnect').click();
  a1.d.getElementById('btnDevice').click(); a1.pulsa('Buscar'); a1.pulsa('JDY-31 Maestro'); a1.entra(M); await micro();
  assert(a1.t('node-name') === 'MAESTRO (POSTE 1)' && a1.t('bt-btn-text') === 'Enlazado' && !a1.ve('bt-modal') &&
         a1.d.getElementById('btnDevice').classList.contains('connected') &&
         /: ROJO, este poste no da paso/.test(a1.t('phase-desc')) && AVISOS.every(id => !a1.ve(id)) &&
         !JERGA.test(textoVisible(a1.d.body)), `T1: conectado: ${a1.t('node-name')} / ${a1.t('bt-btn-text')} / ${a1.t('phase-desc')}`);
  const b1 = app(), bt = b1.w.bluetoothSerial, macs = [];
  b1.d.getElementById('btn-bt-disconnect').click();
  bt.list = (ok) => ok([]); bt.discoverUnpaired = (ok) => ok([{ name: 'SEM-A1-M', id: 'AA:BB', address: 'AA:BB' }]);
  bt.connect = (mac, ok, ko) => { macs.push(mac); if (macs.length === 1) ko('device not paired'); else ok(); };
  b1.d.getElementById('btnDevice').click(); b1.pulsa('Buscar'); b1.pulsa('SEM-A1-M'); await micro();
  const frase = b1.t('aviso-pareja-txt');
  assert(b1.ve('aviso-pareja') && frase === 'No se pudo conectar: este teléfono no está emparejado con el poste.' && frase.length <= 120,
    `T1: sin emparejar, el aviso dice sintoma y que hacer: "${frase}"`);
  b1.pulsa('Emparejar y conectar');
  assert(macs.length === 2 && macs[1] === 'AA:BB' && !b1.ve('aviso-pareja'), `T1: «Emparejar y conectar» repite connect con la misma direccion: ${macs}`);

  // F (D-144, 9.9 (g) 5): cada cartel de la franja, sin clave, UNA frase «<sintoma>: <que hacer>.» <= 120 sin JERGA;
  // medida, limite y causa en su <details> «Detalle». Frase = texto visible del cartel sin botones ni <summary>.
  const fraseF = (a, id) => { const c = a.d.getElementById(id); if (!c || oculto(c)) return null;
    const w = a.d.createTreeWalker(c, 4); let t = '';
    for (let x; (x = w.nextNode());) if (!oculto(x.parentElement) && !x.parentElement.closest('button, summary')) t += ' ' + x.textContent;
    return t.replace(/\s+/g, ' ').trim(); };
  const detalle = (a, id) => { const s = a.d.querySelector(`#${id} details > summary`);
    return s && s.textContent.trim() === 'Detalle' ? s.parentElement.textContent.replace(/\s+/g, ' ') : ''; };
  const franjaOk = (a, id, si, det) => { const f = fraseF(a, id) || '';
    assert(f.length > 0 && f.length <= 120 && /^[^:]+: [^]+\.$/.test(f) && !JERGA.test(f) && si.test(f) && (!det || det.test(detalle(a, id))),
      `F (D-144) ${id}: "${f}" (${f.length}) / detalle "${detalle(a, id).slice(0, 90)}"`); };
  const f1 = app(); f1.entra(M); f1.entra('EVENT,NODE:MAESTRO,ORIGEN:CAMARA_PLUMA,DETALLE:VETO_SOSTENIDO_S:240,HORA:14:31:10');
  franjaOk(f1, 'aviso-camara-pluma', /debajo del brazo/, /240 s[\s\S]*no ve imagen/);
  f1.entra(M); franjaOk(f1, 'aviso-camara-pluma', /ya baj[oó]/, /estuvo retenida 240 s/);
  f1.entra('EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:SUBIDA_DE_TENSION,ARRANQUES:3,PERRO:ARMADO,WDT_MS:2000');
  franjaOk(f1, 'aviso-corte', /SIN LUZ[\s\S]*modo/, /No se sabe cuando fue[\s\S]*Ahora: POSTE 1/);
  const f2 = app(); f2.entra(M.replace('MODO:AUTO', 'MODO:DEGRADADO'));
  f2.entra('EVENT,NODE:MAESTRO,ORIGEN:DEGRADADO,DETALLE:ENLACE_DISPONIBLE,HORA:07:29:34');
  franjaOk(f2, 'aviso-radio-deg', /VOLVER AL MENU/, /07:29:34/);
  const f3 = app(); f3.entra(M); f3.entra('ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_ON_OTRO_OFF_APTO_SI');
  franjaOk(f3, 'degauto-sinradio', /no coinciden/, /este poste ON, el otro OFF/);
  assert(!f3.d.body.classList.contains('admin'), 'F (D-144): «Los postes no coinciden» se ve sin la clave');
  f3.entra(M.replace('ESTADO:ROJO', 'ESTADO:FALLO COM').replace('RF:97', 'RF:0'));
  franjaOk(f3, 'degauto-sinradio', /NO arrancan solos[\s\S]*testigo/, /no tienen la misma eleccion/);
  f3.entra(M); f3.entra('ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_ON_OTRO_ON_APTO_SI');
  assert(!f3.ve('degauto-sinradio'), 'F (D-144): con radio y la misma eleccion, la franja se va');
  f3.entra(M.replace('ESTADO:ROJO', 'ESTADO:FALLO COM').replace('RF:97', 'RF:0'));
  franjaOk(f3, 'degauto-sinradio', /SOLOS en Degradado en aprox\. 5:00/);

  // T3. Exportar el registro, sin clave, por la hoja de compartir (dobles de dom_exportar.js).
  const a3 = app(), llam = { write: [], share: [] };
  a3.w.Capacitor = { isNativePlatform: () => true, Plugins: {
    Filesystem: { writeFile: (x) => { llam.write.push(x); return Promise.resolve({ uri: 'file:///c/' + x.path }); } },
    Share: { share: (x) => { llam.share.push(x); return Promise.resolve({}); } } } };
  a3.pulsa('Eventos'); a3.pulsa('Exportar registro'); await micro();
  assert(llam.write.length === 1 && /^IOTVIAL_SEM-M-01_\d{8}-\d{6}\.txt$/.test(llam.write[0].path) && llam.share.length === 1,
    `T3: sale IOTVIAL_<serie>_<fecha>.txt por la hoja de compartir: ${(llam.write[0] || {}).path}`);

  // T4. Degradado con testigo: sin clave no hay Ajustes; con ella, aviso de paleteros, PIN, SET_RTC y DEG_T.
  const a4 = app(), navAjustes = () => Array.from(a4.d.querySelectorAll('.nav-item')).some(b => !oculto(b) && /Ajustes/.test(b.textContent));
  a4.entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:DEGRADADO,ESTADO:ROJO,T:9,RF:0,RTT:0,BAT:12.9,HORA:14:31:50');
  await new Promise(r => setTimeout(r, 1100));   // js/testigo.js repinta su tarjeta cada segundo
  assert(!navAjustes(), 'T4: sin clave no hay pestaña Ajustes');
  a4.clave();
  assert(a4.t('role-label') === 'Técnico' && navAjustes(), 'T4: con la clave, la pastilla dice Técnico y sale Ajustes');
  a4.pulsa('Ajustes'); a4.d.getElementById('chk-testigo-verificado').checked = true; a4.tramas.length = 0;
  a4.pulsa('Degradado con testigo · POSTE 1');
  assert(a4.ve('aviso-deg-modal') && !a4.tramas.length, 'T4: en Degradado sale antes el aviso de paleteros, sin enviar nada');
  a4.d.getElementById('chk-aviso-deg').click(); a4.pulsa('Cambiar este poste');
  assert(a4.ve('pin-modal'), 'T4: tras el aviso, el PIN'); a4.pin();
  a4.entra('ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,FECHA:2026-11-09,HORA:14:31:55');
  await new Promise(r => setTimeout(r, 1100));
  a4.entra('ACK,CMD:SET_MODO:DEG_T,RESULT:OK'); await new Promise(r => setTimeout(r, 1100));
  const sal = a4.tramas.map(x => x.replace(/^CMD:PIN:\d+:|\r\n$/g, '').split(':').slice(0, 2).join(':'));
  const iR = sal.findIndex(x => x.startsWith('SET_RTC'));
  assert(iR >= 0 && sal.indexOf('SET_MODO:DEG_T') > iR &&
         /^Lleve el testigo al POSTE 2 antes de las \d\d:\d\d:\d\d/.test(a4.t('testigo-estado')),
    `T4: SET_RTC y despues SET_MODO:DEG_T; hora limite para el POSTE 2: ${sal.join(' > ')} / "${a4.t('testigo-estado')}"`);

  // T5. Sacar la barrera de servicio (D-94): plegada en «Averías», y su resumen en ambar si esta fuera.
  const a5 = app(); a5.entra(M);
  assert(!Array.from(a5.d.querySelectorAll('button')).some(b => !oculto(b) && /Sacar la barrera/.test(b.textContent)), 'T5: sin clave no esta a la vista');
  a5.clave(); a5.pulsa('Ajustes');
  assert(!a5.ve('averias-cuerpo'), 'T5: «Averías» nace plegado con todo en servicio');
  a5.pulsa('Averías: cámaras y barrera'); a5.tramas.length = 0; a5.pulsa('Sacar la barrera de servicio');
  if (a5.ve('dialogo-modal')) a5.d.getElementById('btn-dialogo-aceptar').click();
  if (a5.ve('pin-modal')) a5.pin();
  assert(a5.tramas.length === 1 && /SET_BARRERA_SERVICIO:OFF\r\n$/.test(a5.tramas[0]), `T5: sale SET_BARRERA_SERVICIO:OFF: ${a5.tramas}`);
  a5.entra('ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:OK'); a5.entra(M.replace('PLUMA:ABAJO', 'PLUMA:FUERA')); await micro();
  const pl = a5.d.getElementById('pluma-estado'), sm = a5.d.querySelector('#averias > summary');
  assert(/FUERA DE SERVICIO/.test(pl.textContent) && /amber/.test(pl.style.color) && a5.d.getElementById('averias').open &&
         sm.classList.contains('averia-fuera'), `T5: pluma en ambar con su frase y «Averías» abierto en ambar: ${pl.textContent} / ${pl.style.color}`);
}
module.exports.JERGA = JERGA;
