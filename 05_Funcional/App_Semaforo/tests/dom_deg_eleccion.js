// ===== tests/dom_deg_eleccion.js =====
// La eleccion del Degradado automatico (encargo C del orquestador, js/deg_auto_eleccion.js):
// (1) pregunta con dos respuestas; (2) sin radio, desactivada y sin trama; (3) linea en la
// pantalla principal y "Los postes no coinciden"; (5) aviso al conectar, posponible, que vuelve;
// (6) cartel al caer la radio segun la eleccion. Todo sincrono (ver dom_deg_auto.js).
module.exports = async function pruebaDegEleccion(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const d = a.d, w = a.w, E = w.DegAutoEleccion;
  w.clearTimeout(w.DegAuto._timerConsulta);
  const entra = (carga) => w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const st = (modo, est, rf) => entra(`STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:${modo},ESTADO:${est},T:9,RF:${rf},` +
    'RTT:70,BAT:12.9,HORA:14:31:00,ESC:ROJO');
  const consulta = (e, o) => entra(`ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_${e}_OTRO_${o}_APTO_NO`);
  const el = (id) => d.getElementById(id);
  const ve = (id) => !!el(id) && !el(id).hidden;
  const t = (id) => (el(id) || {}).textContent || '';

  // (1) y (3) Con radio: la linea dice la eleccion; los botones son las dos respuestas.
  st('AUTO', 'ROJO', '97');
  consulta('OFF', 'OFF');
  assert(ve('degauto-linea') && t('degauto-linea-txt') === 'Si se cae la radio: AMBAR, ir con testigo' &&
         !ve('degauto-linea-difieren') && !ve('degauto-pide'),
    `Eleccion: OFF/OFF, linea "${t('degauto-linea-txt')}" sin "no coinciden" ni aviso`);
  assert(/^\(a\) Ambar intermitente y voy con el testigo$/.test(t('btn-degauto-off')) &&
         /^\(b\) Entran solos en Degradado a los 5 min sin radio$/.test(t('btn-degauto-on')) &&
         el('btn-degauto-off').getAttribute('aria-pressed') === 'true' && !el('btn-degauto-on').disabled,
    `Eleccion: dos respuestas, (a) marcada: "${t('btn-degauto-off')}" / "${t('btn-degauto-on')}"`);
  assert(/Se elige al montar el equipo, con los dos postes enlazados por radio/.test(t('card-deg-auto')),
    'Eleccion: la tarjeta dice cuando se elige');
  consulta('ON', 'ON');
  assert(t('degauto-linea-txt') === 'Si se cae la radio: DEGRADADO SOLO a los 5 min' && !ve('degauto-linea-difieren'),
    `Eleccion: ON/ON, linea "${t('degauto-linea-txt')}"`);

  // (3) y (5) ESTE y OTRO distintos: rojo en la linea y aviso que pide elegir; se pospone.
  consulta('ON', 'OFF');
  assert(ve('degauto-linea-difieren') && ve('degauto-pide') && /no coinciden/.test(t('degauto-pide-motivo')),
    `Eleccion: ON/OFF, "Los postes no coinciden" y aviso: "${t('degauto-pide-motivo')}"`);
  el('btn-degauto-pide-luego').click();
  assert(!ve('degauto-pide') && ve('degauto-linea-difieren'), 'Eleccion: "Mas tarde" quita el aviso, no la linea');
  // Vuelve en cada conexion: otro poste identificado reinicia el aviso.
  entra('STATUS,NODE:ESCLAVO,SERIE:SEM-E-01,MODO:SUBORDINADO,ESTADO:ROJO,T:--,RF:--,RTT:--,BAT:--,HORA:14:31:01');
  w.clearTimeout(w.DegAuto._timerConsulta);
  consulta('OFF', 'ON');
  assert(ve('degauto-pide'), 'Eleccion: en la siguiente conexion el aviso vuelve');
  // No consta: la consulta no contesta en su plazo.
  w.DegAuto._estado = null; E._pospuesto = false;
  E._conectadoMs = Date.now() - 60000; E.render();
  assert(ve('degauto-pide') && /no ha dicho/.test(t('degauto-pide-motivo')), 'Eleccion: sin respuesta, el aviso pide elegir');
  // Elegir desde el aviso manda SET_DEG_AUTO por el camino de siempre (PIN).
  a.tramas.length = 0;
  el('btn-degauto-pide-off').click();
  ['1', '2', '3', '4'].forEach(k => d.querySelector(`.pin-btn[data-key="${k}"]`).click());
  assert(a.tramas.length === 1 && a.tramas[0] === 'CMD:PIN:1234:SET_DEG_AUTO:0\r\n',
    `Eleccion: (a) desde el aviso manda SET_DEG_AUTO:0: ${a.tramas.join(' | ')}`);
  entra('ACK,CMD:SET_DEG_AUTO,RESULT:OFF');

  // (2) Sin radio: controles desactivados, frase, y ninguna trama aunque se pulse.
  const b = montarAppLimpia();
  const db = b.d, wb = b.w, Eb = wb.DegAutoEleccion;
  wb.clearTimeout(wb.DegAuto._timerConsulta);
  const entraB = (carga) => wb._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const stB = (modo, est, rf) => entraB(`STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:${modo},ESTADO:${est},T:9,RF:${rf},` +
    'RTT:70,BAT:12.9,HORA:14:31:00,ESC:?');
  const eb = (id) => db.getElementById(id);
  const veB = (id) => !!eb(id) && !eb(id).hidden;
  stB('AUTO', 'ROJO', '97');
  entraB('ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_OFF_OTRO_OFF_APTO_NO');
  stB('AUTO', 'FALLO COM', '0');
  assert(eb('btn-degauto-on').disabled && eb('btn-degauto-off').disabled && veB('degauto-sin-radio') &&
         /Sin radio no se puede cambiar: se elige con los dos postes enlazados/.test(eb('degauto-sin-radio').textContent),
    'Eleccion: sin radio los dos botones quedan desactivados con la frase');
  b.tramas.length = 0;
  wb.DegAuto._pulsar('1');
  assert(b.tramas.length === 0, `Eleccion: sin radio no sale SET_DEG_AUTO: ${b.tramas.join(' | ')}`);
  // Control: el ambar pedido por el operario (MODO:AMBAR) con radio medida no es "sin radio".
  assert(!Eb.sinRadio({ node: 'MAESTRO', estadoLuces: 'FALLO COM', modo: 'AMBAR',
                        rfLectura: { medido: true, pct: 97 } }), 'Eleccion: MODO:AMBAR con RF 97 no bloquea');

  // (6) Cartel al caer la radio con (a): ambar y testigo, con boton a esa pantalla.
  assert(veB('degauto-sinradio') && /NO arrancan solos: hay que ir con el testigo/.test(eb('degauto-sinradio-txt').textContent) &&
         veB('btn-degauto-ir-testigo'), `Eleccion: con (a), "${eb('degauto-sinradio-txt').textContent}"`);
  // Con (b): cuenta aprox. de 5 min desde que la app vio caer la radio.
  stB('AUTO', 'ROJO', '97');
  entraB('ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_ON_OTRO_ON_APTO_SI');
  assert(!veB('degauto-sinradio'), 'Eleccion: con radio no hay cartel');
  stB('AUTO', 'FALLO COM', '0');
  const c0 = eb('degauto-sinradio-txt').textContent;
  Eb._caidaMs -= 60000; Eb.render();
  const c1 = eb('degauto-sinradio-txt').textContent;
  assert(/Entran SOLOS en Degradado en aprox\. 5:00$/.test(c0) && /aprox\. 4:00$/.test(c1) && !veB('btn-degauto-ir-testigo'),
    `Eleccion: con (b), cuenta aprox.: "${c0}" -> "${c1}"`);
  entraB('EVENT,NODE:MAESTRO,ORIGEN:DEGRADADO,DETALLE:AUTO_ENTRADA_INICIO_14:40:00,HORA:14:35:00');
  stB('DEGRADADO', 'ROJO', '0');
  assert(eb('degauto-sinradio-txt').textContent === 'En Degradado por reloj, sin radio',
    `Eleccion: en Degradado, "${eb('degauto-sinradio-txt').textContent}"`);
  stB('AUTO', 'ROJO', '95');
  assert(!veB('degauto-sinradio'), 'Eleccion: de vuelta en AUTO con radio, el cartel se va');

  // (6) La cuenta solo con ESTE_ON, OTRO_ON y APTO_SI (SPEC_4 3.ter.bis; firmware degAuto_loop():
  // respaldo_otroApto() y puertaAbierta()). Si no, el firmware se queda en ambar: cartel (a) y motivo.
  const sinCuenta = (carga, motivo, nombre) => {
    stB('AUTO', 'ROJO', '97');
    entraB(`ACK,CMD:CONSULTA_DEG_AUTO,RESULT:${carga}`);
    stB('AUTO', 'FALLO COM', '0');
    const tx = eb('degauto-sinradio-txt').textContent;
    assert(veB('degauto-sinradio') && /NO arrancan solos: hay que ir con el testigo/.test(tx) && motivo.test(tx) &&
           !/Entran SOLOS/.test(tx) && veB('btn-degauto-ir-testigo'),
      `Eleccion: con ${nombre}, ambar y testigo con motivo: "${tx}"`);
  };
  sinCuenta('ESTE_ON_OTRO_OFF_APTO_SI', /Los postes no tienen la misma eleccion/, 'ESTE_ON_OTRO_OFF');
  sinCuenta('ESTE_ON_OTRO_ON_APTO_NO', /Este poste no esta listo para entrar solo/, 'APTO_NO');

  // (6) Cuenta a 0 y 60 s mas sin MODO:DEGRADADO ni AUTO_ENTRADA_INICIO: no entraron (p. ej. sin ECO).
  stB('AUTO', 'ROJO', '97');
  entraB('ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_ON_OTRO_ON_APTO_SI');
  stB('AUTO', 'FALLO COM', '0');
  Eb._caidaMs -= 300000 + 59000; Eb.render();
  const m0 = eb('degauto-sinradio-txt').textContent;
  Eb._caidaMs -= 2000; Eb.render();
  const m1 = eb('degauto-sinradio-txt').textContent;
  assert(/Entran SOLOS/.test(m0) && m1 === 'No entraron solos: quedan en AMBAR. Hay que ir con el testigo' &&
         veB('btn-degauto-ir-testigo'), `Eleccion: cuenta vencida +60 s: "${m0}" -> "${m1}"`);
};
