// ===== tests/dom_cabecera_fase.js =====
// Tres defectos vistos en el telefono el 06/10 (capturas a1_rojo_amar y c2_sin_radio):
// (1) la cabecera decia "(sin enlace)" con la app enlazada; (2) la linea "Si se cae la radio"
// quedaba bajo la barra de pestanas con el cartel sin radio arriba; (3) la frase del centro
// con FALLO COM ocupaba ~9 lineas en la columna. Lo llama test_dom_execution.js.
module.exports = async function pruebaCabeceraFase(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const d = a.d, w = a.w;
  if (w.DegAuto && w.DegAuto._timerConsulta) w.clearTimeout(w.DegAuto._timerConsulta);
  const entra = (carga) => w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const st = (nodo, modo, est, rf, extra = '') => entra(`STATUS,NODE:${nodo},SERIE:SEM-M-01,MODO:${modo},` +
    `ESTADO:${est},T:9,RF:${rf},RTT:70,BAT:12.9,HORA:14:31:00${extra}`);
  const t0 = (id) => d.getElementById(id).textContent;
  // D-82 (SPEC_4 9.9 (d)): #rssi-text nunca pasa de 16 caracteres, en cada lectura.
  const t = (id) => { const v = t0(id); if (id === 'rssi-text') assert(v.length <= 16, `D-82: #rssi-text <= 16: "${v}"`); return v; };

  // (1) La cabecera sigue al enlace: la misma lectura RF de pintarEnlace() y el mismo sinRadio()
  // del cartel (js/deg_auto_eleccion.js). Valores pedidos en el encargo del 06/10.
  st('MAESTRO', 'AUTO', 'ROJO', '97', ',ESC:ROJO,PLUMA:ABAJO');
  assert(t('rssi-text') === '(radio 97 %)', `Cabecera: Maestro con RF:97, "${t('rssi-text')}"`);
  st('MAESTRO', 'AUTO', 'FALLO COM', '0', ',ESC:ROJO,PLUMA:ARRIBA');
  assert(t('rssi-text') === '(sin radio)', `Cabecera: FALLO COM y RF:0, "${t('rssi-text')}"`);
  // SPEC_4 9.9 (c): una frase por ESTADO, por punta; la tabla es el oraculo.
  const TABLA = { 'ROJO': 'ROJO, este poste no da paso', 'ROJO+AMAR': 'Rojo y amarillo: va a abrir el verde',
    'VERDE': 'VERDE, este poste da paso', 'AMARILLO': 'AMARILLO, este poste está cerrando su paso',
    'FALLO COM': 'ámbar y pluma arriba: pasar con precaución' };
  for (const [nodo, modo] of [['MAESTRO', 'AUTO'], ['ESCLAVO', 'SUBORDINADO']]) for (const est of Object.keys(TABLA)) {
    st(nodo, modo, est, nodo === 'MAESTRO' ? '97' : '--', nodo === 'MAESTRO' ? ',ESC:ROJO,PLUMA:ABAJO' : '');
    assert(t0('phase-desc').startsWith(nodo + ': ' + TABLA[est]), `(c) ${nodo} ${est}: "${t0('phase-desc')}"`);
  }
  // Control: el ambar PEDIDO (MODO:AMBAR) con radio medida no es "sin radio".
  st('MAESTRO', 'AMBAR', 'FALLO COM', '97', ',ESC:AMBAR,PLUMA:ARRIBA');
  assert(t('rssi-text') === '(radio 97 %)', `Cabecera: ambar pedido con RF:97, "${t('rssi-text')}"`);
  st('ESCLAVO', 'SUBORDINADO', 'ROJO', '--');
  assert(t('rssi-text') === '(enlazado)', `Cabecera: Esclavo con RF:-- (no lo mide), "${t('rssi-text')}"`);
  d.getElementById('btn-bt-disconnect').click();
  assert(t('rssi-text') === '(sin enlace)', `Cabecera: tras desconectar, "${t('rssi-text')}"`);
  st('MAESTRO', 'AUTO', 'ROJO', '97', ',ESC:ROJO,PLUMA:ABAJO');

  // (3) La frase del centro con FALLO COM: corta (cabe en ~4 renglones de la columna, 60
  // caracteres a ~15 por renglon medidos en la captura c2) y sigue diciendo que se pasa con
  // precaucion. El otro poste ya lo dice su columna (s2-text), no se repite.
  st('MAESTRO', 'AUTO', 'FALLO COM', '0', ',ESC:ROJO,PLUMA:ARRIBA');
  const f = t('phase-desc');
  assert(f.length <= 60 && /precauci/.test(f), `Fase: FALLO COM en <= 60 caracteres y con precaucion: "${f}"`);
  assert(/precauci/.test(t('pluma-estado')) && /ROJO/.test(t('s2-text')),
    `Fase: la pluma y la columna del otro poste siguen diciendolo: "${t('pluma-estado')}" / "${t('s2-text')}"`);
  // D-93 (N-178 M11): sin periodico la app no sabe quien pidio el ambar y no lo afirma. Esta
  // asercion exigia "operario" con MODO:AMBAR a secas: celebraba el defecto (CLAUDE.md 9), se invierte.
  st('MAESTRO', 'AMBAR', 'FALLO COM', '97', ',ESC:AMBAR,PLUMA:ARRIBA');
  const fp = t('phase-desc');
  assert(fp.replace(/^MAESTRO: /, '').length <= 60 && /sin dato/.test(fp) && !/operario/.test(fp) && /precauci/.test(fp),
    `Fase: ambar sin origen leido, corto y sin afirmar quien lo pidio: "${fp}"`);

  // (2) jsdom no calcula posiciones: se comprueba la estructura que lo garantiza. La linea va
  // FUERA de la tarjeta del cruce, encima de ella y debajo del cartel sin radio, asi queda en
  // la mitad alta del telefono (412x915) y no al pie, bajo la barra de pestanas fija.
  const linea = d.getElementById('degauto-linea');
  const tarjeta = d.querySelector('#tab-estado .card-traffic');
  const cartel = d.getElementById('degauto-sinradio');
  const antes = (x, y) => !!(x.compareDocumentPosition(y) & 4);   // DOCUMENT_POSITION_FOLLOWING
  assert(!tarjeta.contains(linea) && antes(linea, tarjeta) && antes(cartel, linea),
    'Linea "Si se cae la radio": fuera de la tarjeta del cruce, entre el cartel sin radio y la tarjeta');

  // (4) D-91..D-93 (SPEC_4 9.11): el origen del ambar del Poste 1 viaja como AMBAR:<o> al final
  // del periodico ENLACE_RF del Maestro; la app lo dice solo cuando lo ha leido. App nueva, para
  // que la cuenta de la bitacora no choque con el tope de 30 lineas de state.events.
  const b = montarAppLimpia(), bd = b.d, bw = b.w;
  if (bw.DegAuto && bw.DegAuto._timerConsulta) bw.clearTimeout(bw.DegAuto._timerConsulta);
  const bEntra = (carga) => bw._btSubscribeCb(`$${carga}*${xor(carga)}
`);
  const bAmbar = () => bEntra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AMBAR,ESTADO:FALLO COM,T:9,RF:97,' +
    'RTT:70,BAT:12.9,HORA:14:31:00,ESC:AMBAR,PLUMA:ARRIBA');
  const per = (rx, o) => bEntra(`EVENT,NODE:MAESTRO,ORIGEN:ENLACE_RF,DETALLE:RX:${rx} OK:5 RUIDO:0 REIN:1` +
    ` AMBAR:${o},HORA:14:31:30`);
  const fase = () => bd.getElementById('phase-desc').textContent;   // la frase de D-93 mide <= 60 sin el nodo
  const largo = (f) => f.replace(/^MAESTRO: /, '').length;
  const lineas = () => bd.querySelectorAll('#event-feed .event-item').length;
  bAmbar();
  const f0 = fase();
  assert(/sin dato/.test(f0) && !/operario/.test(f0), `Origen (i): MODO:AMBAR sin periodico, "${f0}"`);
  const casos = [['APP', /operario/], ['POSTE2', /POSTE 2/], ['ARRANQUE', /reiniciado/]];
  casos.forEach(([o, re], i) => {
    per(10 * (i + 1), o);
    const f = fase();
    assert(re.test(f) && largo(f) <= 60 && /precauci/.test(f), `Origen AMBAR:${o}: "${f}"`);
  });
  assert(largo(f0) <= 60 && /precauci/.test(f0), `Origen sin dato <= 60 y con precaucion: "${f0}"`);
  const l0 = lineas();
  per(40, 'APP'); per(50, 'APP');
  assert(lineas() === l0 + 1, `Origen (v): dos periodicos con el mismo AMBAR:APP, ${lineas() - l0} lineas (1)`);
  bEntra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:9,RF:97,RTT:70,BAT:12.9,HORA:14:32:00,' +
    'ESC:VERDE,PLUMA:ABAJO');
  bAmbar();
  assert(/sin dato/.test(fase()), `Origen (vi): un ambar nuevo no hereda el origen del anterior, "${fase()}"`);

  // (5) D-112 (SPEC_1 §4, fila Menu): la cabecera de MENU dice lo que hacen las luces, leido del ESTADO del $STATUS.
  // Con ESTADO desconocido no afirma una luz: el texto de siempre (control).
  const badge = () => t0('badge-modo');
  for (const [est, rf, txt] of [['ROJO', '97', '☰ MENÚ · ROJO FIJO EN LOS DOS POSTES'],
    ['FALLO COM', '0', '☰ MENÚ · ÁMBAR INTERMITENTE, SIN RADIO'], ['', '0', '☰ MENÚ · ÁMBAR INTERMITENTE, SIN RADIO'],
    ['AMARILLO', '97', '☰ MENÚ · PASANDO A ROJO'], ['ROJO+AMAR', '97', '☰ MENÚ · PASANDO A ROJO']]) {
    st('MAESTRO', 'MENU', est, rf, ',ESC:ROJO,PLUMA:ABAJO');
    assert(badge() === txt, `D-112: MENU con ESTADO:"${est}" -> "${badge()}" (SPEC_1 §4: "${txt}")`);
  }
  st('MAESTRO', 'MENU', 'R1_R2', '97', ',ESC:ROJO,PLUMA:ABAJO');
  assert(!/ROJO FIJO|ÁMBAR|PASANDO/.test(badge()) && /MENÚ/.test(badge()), `D-112 control: ESTADO raro, "${badge()}"`);

  // (6) Modo noche (SPEC_0 §5.10 (a), (g), (i); D-110): cabecera MODO:NOCHE propia y AMBAR_INT/ROJO_INT con frase,
  // pluma arriba y sin caer en «no reconocido». El Esclavo no tiene modo: publica su ESTADO con MODO:SUBORDINADO.
  st('MAESTRO', 'NOCHE', 'AMBAR_INT', '97', ',ESC:AMBAR,PLUMA:ARRIBA');
  assert(/NOCHE/.test(badge()) && /INTERMITENTE|DESTELLO/.test(badge()) && !/RECONOCID/.test(badge()),
    `Noche: cabecera MODO:NOCHE con texto propio, "${badge()}"`);
  const fn = t0('phase-desc');
  assert(/ámbar intermitente/i.test(fn) && /precauci/.test(fn) && fn.length <= 70,
    `Noche: AMBAR_INT con frase, "${t0('phase-desc')}"`);
  assert(/PLUMA ARRIBA/.test(t('pluma-estado')) && /noche/i.test(t('pluma-estado')) && !/AVER/i.test(t('pluma-estado')),
    `Noche: pluma arriba explicada (SPEC_0 §5.10 (a)), "${t('pluma-estado')}"`);
  st('MAESTRO', 'NOCHE', 'ROJO', '97', ',ESC:ROJO,PLUMA:ABAJO');
  assert(/NOCHE/.test(badge()) && /TODO-ROJO/.test(badge()), `Noche: el todo-rojo en la cabecera, "${badge()}"`);
  st('ESCLAVO', 'SUBORDINADO', 'ROJO_INT', '--', ',PLUMA:ARRIBA');
  assert(/rojo intermitente/i.test(t0('phase-desc')) && !/RECONOCID/i.test(t0('phase-desc')),
    `Noche: ROJO_INT del POSTE 2 con frase, "${t0('phase-desc')}"`);
};
