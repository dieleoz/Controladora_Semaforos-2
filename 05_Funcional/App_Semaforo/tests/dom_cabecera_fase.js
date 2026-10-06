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
  const t = (id) => d.getElementById(id).textContent;

  // (1) La cabecera sigue al enlace: la misma lectura RF de pintarEnlace() y el mismo sinRadio()
  // del cartel (js/deg_auto_eleccion.js). Valores pedidos en el encargo del 06/10.
  st('MAESTRO', 'AUTO', 'ROJO', '97', ',ESC:ROJO,PLUMA:ABAJO');
  assert(t('rssi-text') === '(radio 97 %)', `Cabecera: Maestro con RF:97, "${t('rssi-text')}"`);
  st('MAESTRO', 'AUTO', 'FALLO COM', '0', ',ESC:ROJO,PLUMA:ARRIBA');
  assert(t('rssi-text') === '(sin radio entre postes)', `Cabecera: FALLO COM y RF:0, "${t('rssi-text')}"`);
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
  st('MAESTRO', 'AMBAR', 'FALLO COM', '97', ',ESC:AMBAR,PLUMA:ARRIBA');
  const fp = t('phase-desc');
  assert(fp.length <= 60 && /operario/.test(fp) && /precauci/.test(fp), `Fase: ambar pedido corto: "${fp}"`);

  // (2) jsdom no calcula posiciones: se comprueba la estructura que lo garantiza. La linea va
  // FUERA de la tarjeta del cruce, encima de ella y debajo del cartel sin radio, asi queda en
  // la mitad alta del telefono (412x915) y no al pie, bajo la barra de pestanas fija.
  const linea = d.getElementById('degauto-linea');
  const tarjeta = d.querySelector('#tab-estado .card-traffic');
  const cartel = d.getElementById('degauto-sinradio');
  const antes = (x, y) => !!(x.compareDocumentPosition(y) & 4);   // DOCUMENT_POSITION_FOLLOWING
  assert(!tarjeta.contains(linea) && antes(linea, tarjeta) && antes(cartel, linea),
    'Linea "Si se cae la radio": fuera de la tarjeta del cruce, entre el cartel sin radio y la tarjeta');
};
