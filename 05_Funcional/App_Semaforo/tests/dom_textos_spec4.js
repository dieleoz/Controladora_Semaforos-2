// ===== tests/dom_textos_spec4.js =====
// La app contra SPEC_4 / SPEC_6 / SPEC_7: lo que dice al tecnico de pie en la calzada tiene que ser verdad. Cada
// asercion lleva la cita de la SPEC que es su oraculo (no el codigo de la app). Lo llama test_dom_execution.js.
module.exports = async function pruebaTextosSpec4(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const montar = (nodo, modo, serie) => {
    const a = montarAppLimpia();
    a.w.clearTimeout(a.w.DegAuto._timerConsulta);
    a.entra = (c) => a.w._btSubscribeCb(`$${c}*${xor(c)}\n`);
    a.status = (n, s, m, ex) => a.entra(`STATUS,NODE:${n},SERIE:${s},MODO:${m},ESTADO:R1_R2,T:31,RF:97,RTT:70,` +
                                        `BAT:12.9,HORA:14:31:00${ex || ''}`);
    a.ultimo = () => { const e = a.d.querySelector('#event-feed .event-item');
                       return e ? e.textContent.replace(/\s+/g, ' ').trim() : ''; };
    a.pin = () => ['1', '2', '3', '4'].forEach(k => a.d.querySelector(`.pin-btn[data-key="${k}"]`).click());
    a.status(nodo, serie || (nodo === 'MAESTRO' ? 'SEM-M-01' : 'SEM-E-01'), modo);
    return a;
  };
  const dice = (nodo, modo, trama) => { const a = montar(nodo, modo); a.entra(trama);
    const t = a.d.getElementById('toast-msg'); return { t: a.ultimo(), toast: t ? t.textContent : '' }; };
  const GENERICO = /ACEPTADA \(|sin traducir/;
  const M = 'MAESTRO', E = 'ESCLAVO';

  // 1. SPEC_4 §3.ter paso 2 y SPEC_6 A.1.bis: el SET_RTC previo "no bloquea": el testigo sale igual.
  const preparar = (a) => { a.d.getElementById('btn-toggle-role').click();
    if (a.d.getElementById('pin-modal').classList.contains('active')) a.pin();
    a.d.getElementById('chk-testigo-verificado').checked = true; a.tramas.length = 0; };
  const degT = (a) => a.tramas.filter(x => /DEG_T/.test(x));
  {
    const a = montar(M, 'MENU'); preparar(a);
    a.d.getElementById('btn-testigo-maestro').click();
    a.entra('ERR,NODE:PUENTE,CMD:SET_RTC,DESC:SIN_RELOJ_NO_RESPONDE');
    assert(degT(a).length === 1, `1: SET_RTC rechazado por el puente y el testigo SALE igual (SPEC_4 §3.ter 2): ${JSON.stringify(degT(a))}`);
    assert(/SIN_RELOJ|reloj/i.test(a.ultimo()) || a.d.getElementById('event-feed').textContent.includes('SET_RTC'),
      '1: el motivo del rechazo del puente sigue a la vista (aparte)');
  }
  {
    const a = montar(M, 'MENU'); preparar(a);
    const largos = []; const st = a.w.setTimeout;
    a.w.setTimeout = (f, ms, ...r) => { if (ms >= 5000) { largos.push(f); return 0; } return st.call(a.w, f, ms, ...r); };
    a.d.getElementById('btn-testigo-maestro').click();
    largos.forEach(f => f());
    assert(degT(a).length === 1, `1: sin contestar el puente en 10 s el testigo SALE igual (SPEC_6 A.1.bis): ${JSON.stringify(degT(a))}`);
  }

  // 2. SPEC_4 §3.ter.ter 1 / SPEC_2 §7.quater (b): en Degradado el rojo fijo es solo de ESTE poste.
  {
    const a = montar(M, 'DEGRADADO');
    const sub = a.d.getElementById('emergencia-maestro-sub').textContent, hint = a.d.getElementById('emergencia-hint').textContent;
    a.tramas.length = 0; a.d.getElementById('btn-op-emergency').click();
    const linea = a.ultimo();
    assert(!/ambas|dos v[ií]as/i.test(sub + hint + linea) && /otro/i.test(hint + linea),
      `2: en DEGRADADO el boton, su hint y la orden enviada no dicen "ambas vias" y nombran al otro poste: "${sub}" / "${hint}" / "${linea}"`);
    const b = montar(M, 'AUTO');
    assert(/ambas v/i.test(b.d.getElementById('emergencia-maestro-sub').textContent),
      '2: fuera de Degradado el boton sigue diciendo "ambas vias en rojo fijo" (D-45)');
  }

  // 3. SPEC_2 §7.bis y §7.quater (c): el plazo es el despeje en uso (30-255 s): ninguna cifra de 90 s.
  {
    const r = dice(E, 'DEGRADADO', 'ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO');
    assert(!/\b90\b/.test(r.t + ' ' + r.toast), `3: AMBAR_EMERGENCIA|SALIENDO_TODO_ROJO sin cifra de 90 s: "${r.t.slice(0, 170)}" / "${r.toast}"`);
  }

  // 4. SPEC_6 PARTE C: "que hace el tecnico" para las tres causas del limite.
  for (const [causa, esperado] of [['LIMITE_48H', /radio/i], ['SYNC_SIN_FECHA', /radio/i], ['RELOJ_NO_CUENTA', /reloj/i]]) {
    const a = montar(M, 'DEGRADADO');
    a.entra(`ALARM,NODE:MAESTRO,EVENTO:DEGRADADO,CAUSA:${causa},ACCION:CAMBIO_A_AMBAR,HORA:14:36:00`);
    const t = a.ultimo();
    assert(a.w.AvisosEquipo.traducirAlarma({ EVENTO: 'DEGRADADO', CAUSA: causa, NODE: 'MAESTRO' }) !== null && esperado.test(t) && t.length > 160,
      `4: DEGRADADO|${causa} sale con texto de que hacer (SPEC_6 PARTE C): "${t.slice(0, 200)}"`);
  }

  // 5. SPEC_7 §6 y H-2: el badge no afirma una causa; "48 h" no es recalculable (CLAUDE.md §14).
  for (const modo of ['DEGRADADO', 'RENDIDO']) {
    const a = montar(E, modo); const b = a.d.getElementById('badge-modo').textContent;
    assert(!/enlace|vencido|48/i.test(b), `5: badge de ${modo} sin causa inventada: "${b}"`);
  }

  // 6. SPEC_4 §3.1/§3.2: nada cae en el generico ni en "motivo sin traducir".
  const lista = [
    [M, 'MENU', 'ACK,CMD:SET_MODO:AUTO,RESULT:OK'], [M, 'MENU', 'ACK,CMD:SET_MODO:MANUAL,RESULT:OK'],
    [M, 'MENU', 'ACK,CMD:SET_MODO:ALCANCE,RESULT:OK'], [M, 'MENU', 'ACK,CMD:SET_MODO:INTELIGENTE,RESULT:OK'],
    [M, 'MENU', 'ACK,CMD:TEST_LEDS,RESULT:STARTING_6S'], [M, 'MENU', 'ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA'],
    [M, 'INTELIGENTE', 'ACK,CMD:DEMANDA,RESULT:REGISTRADA'],
    [M, 'AUTO', 'ERR,CMD:CAMBIAR_TURNO,DESC:MODO_SIN_CICLO_SALGA_PRIMERO'],
    [M, 'AUTO', 'ERR,CMD:TEST_LEDS,DESC:SIN_ENLACE_AMBAR_NO_SE_PRUEBA'], [M, 'MENU', 'ERR,CMD:TEST_LEDS,DESC:ESPERANDO_ROJO_DEL_ESCLAVO'],
    [M, 'AUTO', 'ERR,CMD:TEST_LEDS,DESC:EN_SERVICIO_PASE_A_MENU'],
  ];
  for (const [n, m, tr] of lista) {
    const r = dice(n, m, tr);
    assert(!GENERICO.test(r.t), `6: ${tr.replace(/^(ACK|ERR),/, '$1 ')} tiene texto propio: "${r.t.slice(0, 120)}"`);
  }
  assert(/hora/i.test(dice(M, 'MENU', 'ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA').t) &&
         /pong/i.test(dice(M, 'MENU', 'ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA').t),
    '6: REINICIAR_RELOJ|CRISTAL_OK_PONGA_LA_HORA dice que ponga la hora');
  assert(!/entro|ya esta en|puesto/i.test(dice(M, 'MENU', 'ACK,CMD:SET_MODO:ALCANCE,RESULT:OK').t),
    '6: ALCANCE|OK no afirma que el modo entro (SPEC_4 §7: contesta OK sin condicion)');

  // 7. SPEC_4 §3.bis (no hay pantalla) y §3.1 (SET_TIEMPOS solo rechaza en AUTOMATICO: el OK no promete rojo).
  assert(!/pantalla/i.test(dice(M, 'MENU', 'ACK,CMD:SET_MODO:MENU,RESULT:OK').t), '7: SET_MODO:MENU|OK no manda a una pantalla que no existe');
  assert(!/en ROJO/i.test(dice(M, 'AMBAR', 'ACK,CMD:SET_TIEMPOS,RESULT:OK').t), '7: SET_TIEMPOS|OK no promete el rojo');

  // 8. SPEC_4 §5: `--` es "no lo se" y `!` es "valor imposible": se pinta `!`.
  {
    const a = montar(M, 'AUTO');
    a.entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:!,RF:!,RTT:!,BAT:12.9,HORA:14:31:05');
    const T = a.d.getElementById('cd-num').textContent, RF = a.d.getElementById('rf-quality').textContent;
    assert(T === '!' && RF === '!', `8: T:! y RF:! se pintan "!" y no "--": T "${T}", RF "${RF}"`);
    a.entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:--,RF:--,RTT:--,BAT:12.9,HORA:14:31:06');
    assert(a.d.getElementById('cd-num').textContent === '--' && a.d.getElementById('rf-quality').textContent === '--',
      '8: T:-- y RF:-- siguen pintandose "--"');
  }

  // 10. SPEC_4 §3.ter.ter 2 y §3.ter 1: los rotulos de los botones.
  {
    const a = montar(M, 'MENU');   // en DEGRADADO el testigo pediria el aviso de paleteros
    const bt = (re) => Array.from(a.d.querySelectorAll('button')).find(b => re.test(b.textContent));
    assert(!!bt(/^Programar salida del Degradado$/), '10: el boton se llama «Programar salida del Degradado»');
    a.d.getElementById('btn-toggle-role').click(); if (a.d.getElementById('pin-modal').classList.contains('active')) a.pin();
    a.d.getElementById('chk-testigo-verificado').checked = true;
    a.d.getElementById('btn-testigo-maestro').click();
    a.entra('ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,FECHA:2026-10-04,HORA:14:31:00');
    a.entra('ACK,CMD:SET_MODO:DEG_T,RESULT:OK');
    a.status(E, 'SEM-E-01', 'SUBORDINADO');
    const t = a.d.getElementById('btn-testigo-esclavo').textContent;
    assert(/^Aplicar testigo guardado — Maestro SEM-M-01, inicio \d\d:\d\d:\d\d$/.test(t), `10: rotulo del Esclavo segun SPEC_4 §3.ter 1: "${t}"`);
  }

  // 11. SPEC_2 §7.quater (e): sin radio, al salir, los dos postes quedan en ambar intermitente.
  assert(/ambar/i.test(dice(M, 'DEGRADADO', 'ACK,CMD:SET_MODO:DEG_FIN,RESULT:PROGRAMADA').t) &&
         /sin radio/i.test(dice(M, 'DEGRADADO', 'ACK,CMD:SET_MODO:DEG_FIN,RESULT:PROGRAMADA').t),
    '11: DEG_FIN|PROGRAMADA avisa de que sin radio los dos postes quedan en ambar intermitente');

  // Comprobacion de lo pedido: un CANCELADA de DEG_FIN pedido desde el Esclavo, ¿borra el registro local del Maestro?
  {
    const a = montar(M, 'DEGRADADO');
    const DF = a.w.DegFin;
    DF.guardar({ estado: 'ACEPTADO', salida: '15:00:00', salidaMs: Date.now() + 600000, serie: 'SEM-M-01' });
    a.status(E, 'SEM-E-01', 'DEGRADADO');
    a.d.getElementById('btn-toggle-role').click(); if (a.d.getElementById('pin-modal').classList.contains('active')) a.pin();
    DF._cancelar();
    if (a.d.getElementById('pin-modal').classList.contains('active')) a.pin();
    const chkAviso = a.d.getElementById('chk-aviso-deg');
    if (a.d.getElementById('aviso-deg-modal').classList.contains('active')) { chkAviso.checked = true;
      chkAviso.dispatchEvent(new a.w.Event('change')); a.d.getElementById('btn-aviso-deg-confirmar').click(); }
    const cancelando = DF._cancelando;
    a.entra('ACK,CMD:SET_MODO:DEG_FIN,RESULT:CANCELADA');
    const reg = DF.leer();
    assert(cancelando && !!reg && reg.estado === 'ACEPTADO' && reg.serie === 'SEM-M-01',
      `CANCELAR pedido desde el Esclavo no borra el registro local del Maestro (SPEC_4 3.ter.ter: el CANCELADA es de ` +
      `ESTE poste): ${JSON.stringify(reg)}`);
    a.status(M, 'SEM-M-01', 'DEGRADADO');
    DF._cancelar();
    if (a.d.getElementById('pin-modal').classList.contains('active')) a.pin();
    if (a.d.getElementById('aviso-deg-modal').classList.contains('active')) { chkAviso.checked = true;
      chkAviso.dispatchEvent(new a.w.Event('change')); a.d.getElementById('btn-aviso-deg-confirmar').click(); }
    a.entra('ACK,CMD:SET_MODO:DEG_FIN,RESULT:CANCELADA');
    assert(DF.leer() === null, 'CANCELAR desde el Maestro si borra su registro local');
  }

  // 12. SPEC_4 §3.bis, D-30 y D-44: no hay LCD, ni botonera, ni mando del gabinete. Los textos que lee el tecnico
  // (literales de app.js y js/*.js, sin comentarios) no los citan.
  {
    const fs = require('fs'), path = require('path');
    const raiz = path.join(__dirname, '..');
    const ficheros = ['app.js'].concat(fs.readdirSync(path.join(raiz, 'js')).filter(f => /\.js$/.test(f)).map(f => 'js/' + f));
    const MALO = /\bLCD\b|pantalla del (equipo|gabinete)|botonera|mando del gabinete|mando de reles|botones de la tarjeta|desde el mando\b|en pantalla salga|pulsador/i;
    const hallazgos = [];
    for (const f of ficheros) {
      fs.readFileSync(path.join(raiz, f), 'utf8').split(/\r?\n/).forEach((l, n) => {
        const t = l.trim();
        if (t.startsWith('//') || t.startsWith('*') || t.startsWith('/*')) return;
        if (MALO.test(t.replace(/\s\/\/.*$/, ''))) hallazgos.push(`${f}:${n + 1}`);
      });
    }
    assert(hallazgos.length === 0, `12: ningun texto del tecnico cita LCD, botonera ni mando del gabinete: ${hallazgos.join(' ')}`);
  }

  // 13. SPEC_6 hueco 2: el mismo $EVENT sirve para 28 dias con testigo y para las ultimas horas del limite sin testigo.
  {
    const ev = (det) => { const a = montar(M, 'DEGRADADO'); a.entra(`EVENT,NODE:MAESTRO,ORIGEN:DEGRADADO,DETALLE:${det},HORA:14:36:00`); return a.ultimo(); };
    const t45 = ev('SYNC:45h AVISO:SI VENCIDA:NO');
    assert(/\b3\b/.test(t45) && /radio/i.test(t45) && /testigo/i.test(t45) && !/28 dias/.test(t45),
      `13: SYNC:45h dice que faltan 3 h (48-45), recuperar la radio o renovar, sin "28 dias": "${t45.slice(0, 220)}"`);
    assert(/\b1\b/.test(ev('SYNC:47h AVISO:SI VENCIDA:NO')) && !/28 dias/.test(ev('SYNC:47h AVISO:SI VENCIDA:NO')), '13: SYNC:47h dice que falta 1 h');
    assert(/28 dias/.test(ev('SYNC:700h AVISO:SI VENCIDA:NO')), '13: con h >= 48 queda el texto de 28 dias');
    assert(/28 dias/.test(ev('SYNC:-- AVISO:SI VENCIDA:NO')), '13: sin horas (SYNC:--) queda el texto de 28 dias');
  }
};
