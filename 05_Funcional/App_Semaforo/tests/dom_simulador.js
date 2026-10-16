// ===== tests/dom_simulador.js =====
// SPEC_4 9.14 (f), (g) (D-120, D-121): el simulador con estado de la practica. No corre sola: la llama
// test_dom_execution.js con SU montarAppLimpia y SU assert. Mide, contra el C++ de 01_Firmware leido en cada corrida:
// constantes con origen, vocabulario en los dos sentidos (con control negativo), las cintas de referencia, el almacen
// de grupos aislado, Salir sin entregas tardias y el menu entero por los botones con el reloj inyectado.
// Lo que no mide: el desborde a 360 px (jsdom no maqueta; herramientas_medir_desborde.js, con navegador), ni el puente
// ESP32 (SPEC_4 3.3), que el simulador no contesta. Verde aqui dice que la app pinta bien, no que el equipo lo haga.
const fs = require('fs');
const path = require('path');
const FW = path.join(__dirname, '..', '..', '..', '01_Firmware');
const APP = path.join(__dirname, '..');

// Un reloj que solo avanza cuando la prueba lo pide (la interfaz que js/simulador.js acepta).
function relojManual(inicioMs) {
  let t = inicioMs, seq = 0;
  const cola = [];
  return {
    ahora: () => t,
    programar(fn, ms) { const h = ++seq; cola.push({ h, en: t + Math.max(0, ms || 0), fn }); return h; },
    cancelar(h) { const i = cola.findIndex(x => x.h === h); if (i >= 0) cola.splice(i, 1); },
    pendientes: () => cola.length,
    avanzar(ms) {
      const fin = t + ms;
      for (;;) {
        cola.sort((a, b) => a.en - b.en || a.h - b.h);
        if (!cola.length || cola[0].en > fin) break;
        const x = cola.shift();
        t = x.en;
        x.fn();
      }
      t = fin;
    }
  };
}

// ---- el C++ de cada punta, sin comentarios ----
const sinComentarios = (s) => s.replace(/\/\*[\s\S]*?\*\//g, ' ').replace(/\/\/[^\n]*/g, ' ');
function fuente(punta) {
  const dir = path.join(FW, punta, 'src');
  return fs.readdirSync(dir).filter(f => f.endsWith('.cpp')).map(f => sinComentarios(fs.readFileSync(path.join(dir, f), 'utf8')));
}
// Literales de cadena; los que siguen con FW_TEXTO / FW_SIN_SELLO llevan un hueco al final.
function literales(codigos) {
  const out = [];
  codigos.forEach(c => {
    const re = /"((?:[^"\\\n]|\\.)*)"(\s*(?:FW_TEXTO|FW_SIN_SELLO)\b)?/g;
    let m;
    while ((m = re.exec(c))) out.push(m[1] + (m[2] ? '%s' : ''));
  });
  return out;
}
const aRegex = (lit) => lit.split(/(%[-0-9.]*l*[sudxXc]|%%)/).map(x =>
  (x === '%%' ? '%' : /^%/.test(x) ? '([^,]*)' : x.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'))).join('');
const DATOS = /^(SERIE|T|RF|RTT|BAT|HORA|FW|REIN|UP|ANTES|TR|NODE)$/;
function vocabulario(punta) {
  const lits = literales(fuente(punta));
  const cabezas = lits.filter(l => /^\$(ACK|ERR|STATUS|EVENT|ALARM),/.test(l))
    .map(l => ({ lit: l, re: new RegExp('^' + aRegex(l)), nombres: (l.match(/(?:^|,)([A-Z_]+):%/g) || []) }));
  const cola = lits.filter(l => /^,[A-Z]+%?[a-z]*:?.*%/.test(l)).map(l => new RegExp('^' + aRegex(l)));
  // Un literal que es solo huecos ("%s") casaria con cualquier cosa: como molde vale si le queda texto fijo.
  const moldes = lits.filter(l => /%/.test(l) && /[A-Za-z]{2}/.test(l.replace(/%[-0-9.]*l*[sudxXc]/g, '')))
    .map(l => new RegExp('^' + aRegex(l) + '$'));
  const enLiteral = (v) => lits.some(l => l.includes(v)) || moldes.some(re => re.test(v));
  return { cabezas, cola, enLiteral };
}
// null si la trama (sin *XX) casa entera con una cabeza del C++ de su punta y cada hueco de vocabulario (CMD,
// RESULT, DESC, MODO, ESTADO, ORIGEN, DETALLE...) es texto de un literal; si no, el motivo.
function juzgar(voc, trama) {
  const t = String(trama).replace(/\*[0-9A-F]{2}$/, '');
  let ultimo = 'ninguna cabeza del C++ casa';
  for (const c of voc.cabezas) {
    const m = c.re.exec(t);
    if (!m) continue;
    let resto = t.slice(m[0].length), mal = null;
    while (resto && !mal) {
      const k = voc.cola.map(r => r.exec(resto)).find(x => x && x[0].length);
      if (!k) mal = 'sobra "' + resto + '"'; else resto = resto.slice(k[0].length);
    }
    const nombres = (c.lit.match(/([A-Z_]+):%|%s/g) || []).map(x => x.replace(/:%$/, ''));
    m.slice(1).forEach((v, i) => {
      const n = nombres[i] || '';
      if (!mal && !DATOS.test(n) && v !== '' && !/^\d+$/.test(v) && !voc.enLiteral(v)) mal = n + ':' + v + ' no esta en el C++';
    });
    if (!mal) return null;
    ultimo = mal;
  }
  return ultimo;
}

module.exports = async function pruebaSimulador(montarAppLimpia, assert) {
  let S = null, err = '';
  try { S = require(path.join(APP, 'js', 'simulador.js')); } catch (e) { err = e.message; }
  assert(!!S && typeof S.crear === 'function', `Simulador: js/simulador.js existe y exporta crear() (${err || 'OK'})`);
  // Sin simulador, uno vacio: cada prueba de abajo sale en rojo por su cuenta, no se salta.
  S = S || { C: { PIN: '1234', STATUS_MS: 2000, DEG_AUTO_ACUSE_MS: 10000 }, ORIGEN: {}, conCrc: (x) => x + '*',
             crear: () => ({ postes: [{}], recibir: () => [], status: () => '', parar() {} }) };
  const T0 = new Date(2026, 9, 15, 10, 0, 0).getTime();
  const flota = () => { try { return JSON.parse(JSON.stringify(require(path.join(APP, 'js', 'practica.js')).FLOTA)); } catch (e) { return []; } };

  // 1. Cada constante con su origen archivo:simbolo, y el valor en la misma linea del simbolo.
  {
    const malas = Object.keys(S.ORIGEN || {}).filter(k => {
      const [rel, sim] = [S.ORIGEN[k].slice(0, S.ORIGEN[k].indexOf(':')), S.ORIGEN[k].slice(S.ORIGEN[k].indexOf(':') + 1)];
      const f = path.join(FW, rel);
      if (!fs.existsSync(f)) return true;
      const v = String(S.C[k]);
      const lineas = sinComentarios(fs.readFileSync(f, 'utf8')).split('\n').filter(l => l.includes(sim));
      return !lineas.some(l => (/^\d+$/.test(v) ? new RegExp('\\b' + v + '(UL|U|L)?\\b') : new RegExp('"' + v + '|' + v)).test(l));
    });
    const n = Object.keys(S.ORIGEN || {}).length;
    assert(n >= 15 && n === Object.keys(S.C || {}).length && malas.length === 0,
      `Simulador: las ${n} constantes tienen origen archivo:simbolo que existe y lleva su valor; malas: ${malas.join(',') || '-'}`);
  }

  // 2. Vocabulario, del simulador al C++: cada trama que emite casa con una cabeza de su punta. Suelo: ABORTADO.
  const VOC = { MAESTRO: vocabulario('Maestro'), ESCLAVO: vocabulario('Esclavo') };
  const sueloOk = VOC.MAESTRO.cabezas.length >= 70 && VOC.ESCLAVO.cabezas.length >= 45;
  assert(sueloOk, `Simulador: ABORTADO si el lector del C++ no ve sus cabezas (Maestro ${VOC.MAESTRO.cabezas.length} >= 70, ` +
    `Esclavo ${VOC.ESCLAVO.cabezas.length} >= 45)`);
  assert(juzgar(VOC.MAESTRO, '$ERR,CMD:SET_TIEMPOS,DESC:INVENTADO_POR_LA_APP*00') !== null &&
         juzgar(VOC.ESCLAVO, '$ACK,CMD:VERSION,RESULT:OK,NODE:ESCLAVO,FW:abc1234,EXTRA:1') !== null &&
         juzgar(VOC.MAESTRO, '$ACK,CMD:SET_TIEMPOS,RESULT:OK*4A') === null,
    'Simulador: control negativo: un DESC: que el C++ no tiene y un campo de mas se rechazan; uno del C++ pasa');
  // Las ordenes del despachador de cada punta, leidas del C++ (C++ -> simulador).
  const ARGS = { 'SET_MODO:DEG_T:': '10:00:00,10:00:30,300,40', 'SET_MODO:DEG_FIN:': 'CANCELAR', 'SET_DEG_AUTO:': '1',
                 'SET_TIEMPOS:': '5,5,30', 'SET_NOCHE:': '1,OFF', 'SET_NOCHE_TR:': '200', 'CMD:HORA_ESP32:': '2026-10-15T10:00:00' };
  const NO_CONTESTA = ['$LATIDO', 'CMD:HORA_ESP32:'];   // bluetooth.cpp: el latido del puente y su hora no se contestan
  function ordenes(punta) {
    const cods = [fs.readFileSync(path.join(FW, punta, 'src', 'bluetooth.cpp'), 'utf8')];
    if (punta === 'Maestro') cods.push(fs.readFileSync(path.join(FW, punta, 'src', 'modo_noche.cpp'), 'utf8'));
    const out = new Set();
    cods.map(sinComentarios).forEach(c => {
      const ini = c.search(/static (void|bool) (procesarComando|ordenEnNoche|modoNoche_orden)|bool modoNoche_orden/);
      const re = /\bstrn?cmp\s*\(\s*(cmd\s*\+\s*4|cmd|accion|a)\s*,\s*"([^"]+)"/g;
      let m;
      re.lastIndex = Math.max(0, ini);
      while ((m = re.exec(c))) {
        const v = m[1].replace(/\s/g, ''), lit = m[2];
        if (lit === 'CMD:' || /^CMD:PIN:\d+:$/.test(lit)) continue;   // las puertas, no ordenes
        const linea = v === 'cmd' ? lit : v === 'cmd+4' ? 'CMD:' + lit : 'CMD:PIN:' + S.C.PIN + ':' + lit;
        out.add(linea);
      }
    });
    return Array.from(out);
  }
  const emitidas = [];
  function contestan(punta, def, lista) {
    const sinRespuesta = [], deMas = [];
    lista.forEach(o => {
      const r = relojManual(T0), em = [];
      const sim = S.crear({ reloj: r, postes: [JSON.parse(JSON.stringify(def))], emitir: (p, t) => em.push(t) });
      const arg = Object.keys(ARGS).find(k => o.endsWith(k));
      const linea = o + (arg ? ARGS[arg] : '');
      const inm = sim.recibir(sim.postes[0], linea + '\r\n');
      r.avanzar(S.C.DEG_AUTO_ACUSE_MS);
      // Contestar «no la conozco» o «PIN malo» a una orden que el C++ acepta (con su PIN) no es contestarla.
      const todas = inm.concat(em).filter(t => /^\$(ACK|ERR)/.test(t) && !/^\$ERR,CMD:(DESCONOCIDO|AUTH_FAILED)/.test(t));
      emitidas.push(...inm.concat(em).map(t => [def.nodo, t]));
      const calla = NO_CONTESTA.some(n => o === n || (n.endsWith(':') && o.startsWith(n)));
      if (calla && todas.length) deMas.push(o); else if (!calla && !todas.length) sinRespuesta.push(o);
    });
    return { sinRespuesta, deMas };
  }
  const F = flota();
  const defM = F.find(p => p.nodo === 'MAESTRO' && p.modo === 'AUTO'), defE = F.find(p => p.nodo === 'ESCLAVO');
  const defNoche = F.find(p => p.modo === 'NOCHE');
  const oM = ordenes('Maestro'), oE = ordenes('Esclavo');
  assert(oM.length >= 28 && oE.length >= 16 && !!defM && !!defE && !!defNoche,
    `Simulador: ABORTADO si no se leen las ordenes del C++ (Maestro ${oM.length} >= 28, Esclavo ${oE.length} >= 16) o falta flota`);
  if (defM && defE && defNoche) {
    const rM = contestan('Maestro', defM, oM), rN = contestan('Maestro', defNoche, oM), rE = contestan('Esclavo', defE, oE);
    const falta = rM.sinRespuesta.concat(rN.sinRespuesta, rE.sinRespuesta), sobra = rM.deMas.concat(rN.deMas, rE.deMas);
    assert(falta.length === 0 && sobra.length === 0,
      `Simulador: cada orden del despachador del C++ tiene respuesta (${oM.length} Maestro, ${oE.length} Esclavo), y las ` +
      `que el firmware no contesta tampoco aqui; sin respuesta: ${falta.join(' ') || '-'}; de mas: ${sobra.join(' ') || '-'}`);
    // Barrido: en cada poste de la flota, cada orden de su punta con argumentos buenos y malos, dos veces seguidas
    // (las ramas que dependen de lo anterior: REARMADO, SIN_CAMBIO, YA_VUELVE_AL_MENU...).
    const VARIANTES = { 'SET_MODO:DEG_T:': ['10:00:00,10:00:30,300,40', '10:00:00,10:00:30,301,40', '10:00:00,10:00:30,300,5',
                                            '09:00:00,10:00:30,300,40', 'x'],
                        'SET_MODO:DEG_FIN:': ['CANCELAR', '10:00:00,10:30:00', '09:00:00,10:30:00', 'x'],
                        'SET_DEG_AUTO:': ['1', '0', '7'], 'SET_TIEMPOS:': ['5,5,30', '99,5,30', 'x'],
                        'SET_NOCHE:': ['1,OFF', '1,LMXJVSD,2200,0500,R', '9,OFF', '1,LMXJVSD,2500,0500,A', 'x'],
                        'SET_NOCHE_TR:': ['200', '5', 'x'] };
    F.forEach(def => {
      const r = relojManual(T0), em = [];
      const sim = S.crear({ reloj: r, postes: [JSON.parse(JSON.stringify(def))], emitir: (p, t) => em.push([def.nodo, t]) });
      (def.nodo === 'MAESTRO' ? oM : oE).forEach(o => {
        const k = Object.keys(VARIANTES).find(x => o.endsWith(x));
        (k ? VARIANTES[k] : ['']).forEach(v => [1, 2].forEach(() =>
          sim.recibir(sim.postes[0], o + v).forEach(t => emitidas.push([def.nodo, t]))));
        r.avanzar(S.C.STATUS_MS);
      });
      r.avanzar(60000);
      emitidas.push(...em);
    });
    // Cada poste de la flota, su $STATUS a lo largo de diez minutos y sus $EVENT.
    const r = relojManual(T0), em = [];
    const sim = S.crear({ reloj: r, postes: F, emitir: (p, t) => em.push([p.nodo, t]) });
    for (let i = 0; i < 300; i++) { sim.postes.forEach(p => emitidas.push([p.nodo, sim.status(p)])); r.avanzar(2000); }
    emitidas.push(...em);
    const malas = emitidas.map(([n, t]) => [t, juzgar(VOC[n], t)]).filter(x => x[1]);
    const tipos = new Set(emitidas.map(([, t]) => t.split(',')[0]));
    assert(sueloOk && malas.length === 0 && ['$ACK', '$ERR', '$STATUS', '$EVENT'].every(x => tipos.has(x)),
      `Simulador: las ${emitidas.length} tramas que emite casan con una cabeza del C++ de su punta; malas: ` +
      `${malas.slice(0, 3).map(x => x[0] + ' (' + x[1] + ')').join(' | ') || '-'}`);
    const crcMal = emitidas.filter(([, t]) => S.conCrc(t.replace(/\*[0-9A-F]{2}$/, '')) !== t);
    assert(emitidas.length > 0 && crcMal.length === 0, `Simulador: cada trama lleva el *XX de enviarTramaConCrc(): ${crcMal.length} malas`);
  }

  // 3. Las cintas de referencia (D-121): con el simulador en el estado de la cinta, cada orden con respuesta grabada
  //    da los mismos campos (salvo HORA, T, RTT y contadores) y cada $STATUS los mismos nombres en el mismo orden.
  {
    const cinta = (n) => new Function(fs.readFileSync(path.join(APP, 'js', 'practica_cinta_' + n + '.js'), 'utf8') +
                                      '; return PRACTICA_CINTA_' + n + ';')();
    const campos = (t) => t.replace(/\*[0-9A-F]{2}$/, '').split(',').slice(1).map(c => [c.split(':')[0], c.slice(c.indexOf(':') + 1)]);
    const SIN_PIN = ['FORZAR_ROJO', 'AMBAR_EMERGENCIA', 'SET_MODO:MENU', 'SET_MODO:ALCANCE', 'VERSION', 'CONSULTA_DEG_AUTO',
                     'CONSULTA_DEG_FIN', 'CONSULTA_NOCHE', 'REINICIOS'];   // los de app.js, que es quien las envia
    const VARIAN = /^(HORA|T|RTT|REIN|UP|ANTES|DETALLE)$/;
    const difieren = [], nombres = [];
    let ordenesVistas = 0, statusVistos = 0, enteros = 0;
    [1, 2, 3, 4].forEach(n => {
      const c = cinta(n), r = relojManual(T0);
      const st0 = campos(c.tramas.find(t => /^\$STATUS/.test(t)));
      const def = { nodo: c.nodo, serie: c.serie, fw: c.firmware, direccion: 'CINTA', modo: st0.find(x => x[0] === 'MODO')[1] };
      const sim = S.crear({ reloj: r, postes: [def] });
      const p = sim.postes[0];
      const sincronizar = (st) => {
        const v = (k) => (st.find(x => x[0] === k) || [])[1];
        if (v('MODO') !== p.modo) { p.modo = v('MODO'); p.degSaliendo = false; p.rojoFijo = false; }
        p.rf = v('RF') === '--' ? null : parseInt(v('RF'), 10); p.rtt = parseInt(v('RTT'), 10) || null;
        p.radio = c.nodo === 'MAESTRO' ? !!p.rf && v('ESTADO') !== 'FALLO COM' : v('ESTADO') !== 'FALLO COM';
        const h = (v('HORA') || '').split(':');
        if (h.length === 3) r.avanzar(Math.max(0, new Date(2026, 9, 15, +h[0], +h[1], +h[2]).getTime() - r.ahora()));
      };
      const orden = (t) => { const k = /CMD:([^,*]+)/.exec(t)[1];
        return SIN_PIN.indexOf(k) >= 0 ? 'CMD:' + k : 'CMD:PIN:' + S.C.PIN + ':' + k; };
      const comparar = (grabada, dada) => {
        ordenesVistas++;
        const a = campos(grabada), b = campos(dada || '');
        const ok = a.length === b.length && a.every((x, i) => x[0] === b[i][0] && (VARIAN.test(x[0]) || x[1] === b[i][1]));
        if (!ok) difieren.push('cinta ' + n + ': ' + grabada + ' <> ' + dada);
        if (ok && grabada.replace(/\*..$/, '') === String(dada).replace(/\*..$/, '') && grabada !== dada) difieren.push('*XX ' + grabada);
      };
      (c.respuestas || []).filter(t => /CMD:VERSION/.test(t)).forEach(t => comparar(t, sim.recibir(p, orden(t))[0]));
      c.tramas.forEach(t => {
        if (/^\$STATUS/.test(t)) {
          statusVistos++;
          sincronizar(campos(t));
          const dada = sim.status(p);
          const na = campos(t).map(x => x[0]).join(','), nb = campos(dada).map(x => x[0]).join(',');
          if (na !== nb) nombres.push('cinta ' + n + ': ' + na + ' <> ' + nb);
          if (dada === t) enteros++;
        } else if (/^\$(ACK|ERR),CMD:/.test(t)) comparar(t, sim.recibir(p, orden(t))[0]);
      });
    });
    assert(ordenesVistas >= 8 && difieren.length === 0,
      `Simulador: las ${ordenesVistas} respuestas grabadas en las cintas las da igual el simulador en su estado; ` +
      `difieren: ${difieren.slice(0, 3).join(' | ') || '-'}`);
    assert(statusVistos >= 100 && nombres.length === 0 && enteros > 0,
      `Simulador: los ${statusVistos} $STATUS de las cintas tienen los mismos campos en el mismo orden (${enteros} ` +
      `iguales con su *XX); distintos: ${nombres.slice(0, 2).join(' | ') || '-'}`);
  }

  // 4. En la app: almacen de grupos aislado, Salir sin entregas tardias y el menu entero por los botones.
  const montar = () => {
    const a = montarAppLimpia();
    a.el = (id) => a.d.getElementById(id);
    a.pulsa = (id) => { if (a.el(id)) a.el(id).click(); };
    a.activo = (id) => !!a.el(id) && a.el(id).classList.contains('active');
    a.pin = () => ['1', '2', '3', '4'].forEach(k => a.d.querySelector(`.pin-btn[data-key="${k}"]`).click());
    a.pulsa('btn-bt-disconnect');
    a.pulsa('btn-toggle-role');
    if (a.el('clave-entrada')) { a.el('clave-entrada').value = '1234'; a.pulsa('btn-clave-entrar'); }
    a.tramas.length = 0;
    a.r = relojManual(Date.now());   // la app pone `ahora` con el reloj del telefono (testigo, salida)
    if (a.w.Practica) a.w.Practica.reloj = a.r;
    a.oidas = [];
    a.enlazar = (mac) => {
      const fila = a.d.querySelector(`#bt-modal .bt-device-item[data-mac="${mac}"]`);
      if (fila) fila.click();
      const P = a.w.PuertoPractica;
      if (P && P._cb) { const cb = P._cb; P._cb = (t) => { a.oidas.push(t.trim()); cb(t); }; }
      a.r.avanzar(S.C.STATUS_MS);
      return !!fila;
    };
    return a;
  };
  const P = (a) => a.w.Practica || { entrar() { return false; }, salir() {}, FLOTA: [] };
  const mac = (n) => '02:00:00:00:00:' + String(n).padStart(2, '0');
  {
    const a = montar();
    const real = () => a.w.localStorage.getItem('iot_vial_grupos_v1');
    const antes = real();
    P(a).entrar();
    a.enlazar(mac(1));
    const cab = a.el('current-site-name') ? a.el('current-site-name').textContent : '';
    a.pulsa('btn-bt-disconnect'); a.pulsa('btnDevice');
    a.enlazar(mac(11));
    a.pulsa('btn-select-site'); a.pulsa('btn-open-add-site');
    ['Via 9', 'Km 99', ''].forEach(v => { if (a.el('dialogo-entrada')) a.el('dialogo-entrada').value = v; a.pulsa('btn-dialogo-aceptar'); });
    const cab2 = a.el('current-site-name') ? a.el('current-site-name').textContent : '';
    assert(P(a).activa && P(a).activa() && /Via 1 · Km 20/.test(cab) && /Via 9 · Km 99/.test(cab2) && real() === antes,
      `Simulador: la flota trae sus grupos ("${cab.trim()}") y matricular en practica ("${cab2.trim()}") deja el almacen ` +
      `real igual byte a byte (${String(antes).length} B)`);
    a.pulsa('btn-bt-disconnect'); a.pulsa('btnDevice');
    a.enlazar(mac(10));
    const feed = Array.from(a.d.querySelectorAll('#event-feed .event-item')).map(x => x.textContent).join(' || ');
    assert(/Serie cambiada/.test(feed) && /FW|firmware/i.test(feed),
      `Simulador: el ESCLAVO de PR 45 da los avisos de serie cambiada y FW distinto: ${feed.slice(0, 200)}`);
    P(a).salir();
    const cab3 = a.el('current-site-name') ? a.el('current-site-name').textContent : '';
    assert(/SIN GRUPO/.test(cab3), `Simulador: tras Salir la cabecera no lleva el grupo de la practica: "${cab3.trim()}"`);
  }
  {
    // Como el firmware y la cinta (practica_cinta_1.js): primero el $ACK de la orden y despues su $EVENT.
    const a = montar();
    P(a).entrar(); a.enlazar(mac(1));
    const n = a.oidas.length;
    if (a.w.PuertoPractica) a.w.PuertoPractica.write('CMD:PIN:' + S.C.PIN + ':SET_MODO:MANUAL\r\n');
    a.r.avanzar(1000);
    const nuevas = a.oidas.slice(n).filter(t => /^\$(ACK|EVENT)/.test(t)).map(t => t.split(',')[0]);
    assert(nuevas[0] === '$ACK' && nuevas.includes('$EVENT'), `Simulador: el $ACK llega antes que su $EVENT: ${nuevas.join(' ')}`);
    P(a).salir();
  }
  {
    const a = montar();
    P(a).entrar();
    a.enlazar(mac(1));
    a.pulsa('btn-leer-version');
    P(a).salir();
    const n = a.oidas.length;
    a.r.avanzar(60000);
    const sinPendientes = a.r.pendientes() === 0 && a.oidas.length === n;
    P(a).entrar(); a.enlazar(mac(1));
    a.pulsa('btn-leer-version'); P(a).salir(); P(a).entrar(); a.enlazar(mac(3));
    a.r.avanzar(1000);
    assert(n >= 1 && sinPendientes && !a.oidas.some(t => /CMD:VERSION/.test(t)) && a.tramas.length === 0,
      `Simulador: tras Salir el reloj avanzado no entrega nada (${a.oidas.length - n} tras salir), tampoco entrando y ` +
      `saliendo dentro de RESPUESTA_MS, y el puerto real no recibe nada (${a.tramas.length})`);
    P(a).salir();
  }
  {
    // El menu entero por los botones, sin enlace real: cada orden que sale tiene respuesta del simulador.
    const a = montar();
    const escritas = [];
    P(a).entrar();
    const Pp = a.w.PuertoPractica;
    if (Pp) { const w = Pp.write.bind(Pp); Pp.write = (d, ok) => { escritas.push(String(d).trim()); w(d, ok); }; }
    if (a.w.Testigo) a.w.Testigo.ESPERA_RTC_MS = 50;
    const pulsar = (id) => {
      a.pulsa(id);
      for (let i = 0; i < 3; i++) {
        if (a.activo('via-modal')) a.pulsa('btn-via-confirmar');
        if (a.activo('aviso-deg-modal')) {   // los paleteros: la casilla habilita el boton
          a.el('chk-aviso-deg').checked = true;
          a.el('chk-aviso-deg').dispatchEvent(new a.w.Event('change', { bubbles: true }));
          a.pulsa('btn-aviso-deg-confirmar');
        }
        if (a.activo('dialogo-modal')) a.pulsa('btn-dialogo-aceptar');
        if (a.activo('pin-modal')) { a.pin(); a.pulsa(id); }
      }
      a.r.avanzar(200);
    };
    const pasos = (m, ids) => { a.pulsa('btn-bt-disconnect'); a.pulsa('btnDevice'); a.enlazar(m); ids.forEach(pulsar); };
    pasos(mac(1), ['btn-degauto-on', 'btn-op-menu', 'btn-modo-alcance', 'btn-op-menu', 'btn-start-test-leds',
                   'btn-leer-version', 'btn-op-auto', 'btn-op-step', 'btn-op-amber', 'btn-op-emergency', 'btn-op-menu']);
    a.pulsa('tab-btn-tiempos');
    a.el('num-tiempo-verde').value = '5'; a.el('num-tiempo-rojo').value = '5'; a.el('num-tiempo-despeje').value = '30';
    pulsar('btn-aplicar-tiempos');
    pasos(mac(3), ['btn-noche-f2-apagar']);
    a.el('num-noche-distancia').value = '500';
    a.el('num-noche-distancia').dispatchEvent(new a.w.Event('input', { bubbles: true }));
    a.el('num-noche-tr').value = '200';
    ['btn-noche-tr', 'btn-op-menu'].forEach(pulsar);
    pasos(mac(5), []);
    if (a.el('chk-testigo-verificado')) a.el('chk-testigo-verificado').checked = true;
    pulsar('btn-testigo-maestro');
    await new Promise(r => setTimeout(r, 300));   // ESPERA_RTC_MS (50 ms aqui) es del reloj de la app, no del simulador
    a.r.avanzar(200);
    pasos(mac(6), ['btn-op-ambar-emergencia', 'btn-op-cancelar-ambar']);
    const puente = /^CMD:(PIN:\d+:)?(LEER_RTC|SET_RTC|LOG)/;
    const alSim = escritas.filter(o => !puente.test(o));
    const cmdDe = (o) => o.replace(/^CMD:(PIN:\d+:)?/, '').replace(/^MANUAL:/, '').split(':').slice(0, 2).join(':');
    const sinRespuesta = alSim.filter(o => !a.oidas.some(t => /^\$(ACK|ERR),CMD:/.test(t) &&
      (t.indexOf('CMD:' + cmdDe(o)) >= 0 || /AUTH_FAILED|DESCONOCIDO/.test(t) ||
       (/SET_NOCHE/.test(o) && /CMD:SET_NOCHE/.test(t)) || (/DEG_T/.test(o) && /DEG_T/.test(t)))));
    const hechas = ['SET_MODO:MENU', 'SET_MODO:ALCANCE', 'TEST_LEDS', 'VERSION', 'SET_MODO:AUTO', 'CAMBIAR_TURNO',
                    'SET_MODO:AMBAR', 'FORZAR_ROJO', 'SET_TIEMPOS', 'SET_NOCHE', 'SET_NOCHE_TR', 'SET_DEG_AUTO',
                    'SET_MODO:DEG_T', 'AMBAR_EMERGENCIA', 'CANCELAR_AMBAR'].filter(k => !alSim.some(o => o.includes(k)));
    assert(alSim.length >= 15 && hechas.length === 0 && sinRespuesta.length === 0 && a.tramas.length === 0,
      `Simulador: el menu entero por los botones (${alSim.length} ordenes) sin enlace real; sin pulsar: ` +
      `${hechas.join(',') || '-'}; sin respuesta: ${sinRespuesta.join(' | ') || '-'}; al puerto real: ${a.tramas.length}`);
    P(a).salir();
  }
};
// El juez, a mano desde node para mirar una trama suelta: require('./tests/dom_simulador.js').juzgar(...).
module.exports.juzgar = juzgar;
module.exports.vocabulario = vocabulario;
