// ===== tests/dom_deg_fin.js =====
// D-51 corregida y D-52 en la APP (SPEC_2 7.quater (i), SPEC_4 3.1, 3.ter.ter): pruebas ANTERIORES al
// codigo, en rojo hasta construirlo. Los textos esperados son los de SPEC_4 3.ter.ter, no los de app.js.
// NO SE HAN CORRIDO donde se escribieron (worktree sin node_modules, CLAUDE.md 17): `node --check` solo.
// La SPEC no da ids de DOM del boton ni del formulario: se busca el boton por su rotulo de la SPEC.
// Lo llama test_dom_execution.js, como dom_version.js.
module.exports = async function pruebaDegFin(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const d = a.d;
  a.w.clearTimeout(a.w.DegAuto._timerConsulta);
  const entra = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const ultimo = () => { const e = d.querySelector('#event-feed .event-item'); return e ? e.textContent : ''; };
  const boton = (re) => Array.from(d.querySelectorAll('button')).find(b => re.test(b.textContent));
  const pagina = () => d.body.textContent;
  const status = (nodo, serie, modo) => entra(`STATUS,NODE:${nodo},SERIE:${serie},MODO:${modo},ESTADO:R1_R2,T:31,` +
    'RF:0,RTT:0,BAT:12.9,HORA:14:31:00');
  const pin = () => ['1', '2', '3', '4'].forEach(k => d.querySelector(`.pin-btn[data-key="${k}"]`).click());
  const DEG_FIN = /^CMD:PIN:1234:SET_MODO:DEG_FIN:\d\d:\d\d:\d\d,\d\d:\d\d:\d\d\r\n$/;

  // A1: el acuse de FORZAR_ROJO en Degradado. La clave tiene que EXISTIR (si no, saldria el generico "ACEPTADA (...)").
  status('MAESTRO', 'SEM-M-01', 'DEGRADADO');
  entra('ACK,CMD:FORZAR_ROJO,RESULT:ROJO_FIJO_EN_ESTE_POSTE');
  const t1 = ultimo();
  assert(!/ACEPTADA \(ROJO_FIJO/.test(t1) && /este poste/i.test(t1) && /otro poste sigue alternando/i.test(t1) &&
         !/no vuelve a alternar/i.test(t1) && !/men[uú]/i.test(t1),
    `A1: FORZAR_ROJO|ROJO_FIJO_EN_ESTE_POSTE dice "este poste" y "el otro sigue alternando", ni "no vuelve a ` +
    `alternar" ni "menu": "${t1.slice(0, 200)}"`);
  entra('ACK,CMD:FORZAR_ROJO,RESULT:YA_EN_ROJO_FIJO');
  assert(!/ACEPTADA \(YA_EN/.test(ultimo()) && /ya estaba en rojo fijo/i.test(ultimo()),
    `A1: YA_EN_ROJO_FIJO dice "ya estaba en rojo fijo; nada cambia": "${ultimo().slice(0, 160)}"`);

  // A2 + A3: el boton "Programar salida" existe en las DOS pantallas y manda DEG_FIN con el vale UNA vez.
  const relojReal = a.w.Date; let corrimientoMs = 0;
  a.w.Date = class extends relojReal {
    constructor(...x) { if (x.length) super(...x); else super(relojReal.now() + corrimientoMs); }
    static now() { return relojReal.now() + corrimientoMs; }
  };
  const programar = () => boton(/Programar salida/i);
  assert(!!programar(), 'A3: en la pantalla del Maestro existe el boton "Programar salida" (D-52, SPEC_4 3.ter.ter 2)');
  if (programar()) {
    a.tramas.length = 0;
    programar().click();
    if (d.getElementById('pin-modal').classList.contains('active')) pin();
    const modal = d.getElementById('aviso-deg-modal');
    assert(modal.classList.contains('active') && a.tramas.length === 0,
      `A2: con MODO:DEGRADADO el aviso de paleteros abre y NO sale nada: ${a.tramas.join(' | ')}`);
    const chk = d.getElementById('chk-aviso-deg');
    chk.checked = true; chk.dispatchEvent(new a.w.Event('change'));
    corrimientoMs = 5000;   // el `ahora` de la 2.a pasada es distinto: el vale se casa por NOMBRE de orden
    d.getElementById('btn-aviso-deg-confirmar').click();
    assert(!modal.classList.contains('active') && a.tramas.length === 1 && DEG_FIN.test(a.tramas[0]),
      `A2: confirmar con otro \`ahora\` envia UNA SET_MODO:DEG_FIN y no vuelve a preguntar: ${a.tramas.join(' | ')}`);
    const m = (a.tramas[0] || '').match(/DEG_FIN:(\d\d:\d\d:\d\d),(\d\d:\d\d:\d\d)/);
    assert(!!m && m[1] !== '' && m[1] > '00:00:00',
      `A3: la trama lleva ahora,salida con \`ahora\` releido en el envio (5 s mas tarde que al pulsar): ${a.tramas[0]}`);
    entra('ACK,CMD:SET_MODO:DEG_FIN,RESULT:PROGRAMADA');
    status('ESCLAVO', 'SEM-E-01', 'DEGRADADO');
    assert(!!boton(/Programar salida/i),
      'A3: en la pantalla del Esclavo tambien existe el boton "Programar salida"');
    const guardada = boton(/Aplicar salida guardada/i);
    assert(!!m && !!guardada && guardada.textContent.includes('SEM-M-01') && guardada.textContent.includes(m[2]),
      'A3: el Esclavo ofrece "Aplicar salida guardada" con la serie del Maestro y la salida (SEM-M-01 y ' +
      `${m && m[2]}): "${guardada ? guardada.textContent : '(sin boton)'}"`);
  }
  a.w.Date = relojReal;
  status('MAESTRO', 'SEM-M-01', 'DEGRADADO');

  // A4: cada RESULT/DESC de DEG_FIN, MENU y la alarma de rojo total se traducen (nada "ACEPTADA (" ni "sin traducir").
  for (const r of ['PROGRAMADA', 'REPROGRAMADA', 'PROGRAMADA_SIN_RESPALDO', 'CANCELADA']) {
    entra(`ACK,CMD:SET_MODO:DEG_FIN,RESULT:${r}`);
    assert(!/ACEPTADA \(/.test(ultimo()), `A4: DEG_FIN|${r} se traduce: "${ultimo().slice(0, 120)}"`);
  }
  const motivos = ['FORMATO_INVALIDO', 'No esta en Degradado', 'Ya esta saliendo', 'Falta: reloj sin poner en hora',
    'Ahora no coincide', 'Salida ya vencida', 'Salida antes del inicio', 'En verde: repita en rojo',
    'No se pudo guardar la salida', 'No hay salida programada', 'Cancele antes la salida programada'];
  for (const desc of motivos) {
    entra(`ERR,CMD:SET_MODO:DEG_FIN,DESC:${desc}`);
    assert(!/sin traducir/.test(ultimo()), `A4: DEG_FIN $ERR "${desc}" se traduce: "${ultimo().slice(0, 140)}"`);
  }
  entra('ERR,CMD:SET_MODO:DEG_FIN,DESC:Salida ya vencida');
  assert(/reprograme la salida en el Maestro/i.test(ultimo()) && !/reintent|repita|vuelva a mandar/i.test(ultimo()),
    `A4: "Salida ya vencida" dice reprogramar en el Maestro y NO ofrece reintentar: "${ultimo().slice(0, 200)}"`);
  // B2: el literal de «Salida ya vencida» del ESCLAVO se lee de SU fuente (no se escribe aqui) y la app lo traduce igual.
  const srcE = require('fs').readFileSync(require('path').join(__dirname, '../../../01_Firmware/Esclavo/src/modo_degradado.cpp'), 'utf8');
  const mE = srcE.match(/case MDF_SALIDA_VENCIDA:\s*return "([^"]*)";/);
  assert(!!mE && mE[1] === 'Salida ya vencida',
    `A4: el Esclavo rechaza con el MISMO literal que el Maestro, "Salida ya vencida": "${mE ? mE[1] : '(sin literal)'}"`);
  if (mE) {
    entra(`ERR,CMD:SET_MODO:DEG_FIN,DESC:${mE[1]}`);
    assert(!/sin traducir/.test(ultimo()) && /reprograme la salida en el Maestro/i.test(ultimo()),
      `A4: el literal del Esclavo ("${mE[1]}") se traduce y dice reprogramar en el Maestro: "${ultimo().slice(0, 200)}"`);
  }
  entra('ACK,CMD:SET_MODO:MENU,RESULT:OK_REANUDACION_CANCELADA');
  assert(!/ACEPTADA \(/.test(ultimo()) && /NO se reanudar/i.test(ultimo()),
    `A4: MENU|OK_REANUDACION_CANCELADA dice que el degradado NO se reanudara: "${ultimo().slice(0, 160)}"`);
  entra('ALARM,NODE:MAESTRO,EVENTO:DEGRADADO,CAUSA:ROJO_TOTAL,RF:--,RTT:--,SINRESP:3,ACCION:ROJO_FIJO,HORA:14:36:00');
  assert(/persona/i.test(ultimo()) && /otro poste sigue alternando/i.test(ultimo()),
    `A4: $ALARM CAUSA:ROJO_TOTAL dice "puesto por una persona; el otro poste sigue alternando": ` +
    `"${ultimo().slice(0, 200)}"`);
  entra('ACK,CMD:CONSULTA_DEG_FIN,RESULT:SALE_143000_FALTAN_120S_SIN_RESPALDO');
  assert(/sale a 14:30:00, faltan 120 s/.test(pagina()) && /sin respaldo/i.test(pagina()),
    'A4: CONSULTA_DEG_FIN pinta "sale a 14:30:00, faltan 120 s" con la marca "sin respaldo" en la tarjeta');
  entra('ACK,CMD:CONSULTA_DEG_FIN,RESULT:NINGUNA');
  assert(!/sale a 14:30:00/.test(pagina()), 'A4: CONSULTA_DEG_FIN|NINGUNA borra la cuenta atras de la tarjeta');
  assert(/salida programada en el Maestro; FALTA el Esclavo/i.test(pagina()),
    'A4: si la app programo el Maestro y no consta el Esclavo, el cartel dice "salida programada en el Maestro; ' +
    'FALTA el Esclavo" y se queda fijo');
};
