// ===== tests/dom_aviso_radio_deg.js =====
// AVISO DE RADIO EN DEGRADADO. El firmware repite cada 60 s, mientras oye a la otra punta
// estando en Degradado, $EVENT,NODE:..,ORIGEN:DEGRADADO,DETALLE:ENLACE_DISPONIBLE,HORA:..
// (cinta de campo evidencia/011020260934/IOTVIAL_4D2007_*.txt, *40). La app tiene que dejar
// un CARTEL fijo con la hora del ultimo aviso, y toast y linea solo la primera vez por
// conexion. Lo llama test_dom_execution.js, como dom_aviso_corte.js.
module.exports = async function pruebaAvisoRadioDeg(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const VUELTA = (h) => 'EVENT,NODE:MAESTRO,ORIGEN:DEGRADADO,DETALLE:ENLACE_DISPONIBLE,HORA:' + h;
  assert(xor(VUELTA('07:29:34')) === '40', `RadioDeg: la trama de prueba es la de campo (*40): *${xor(VUELTA('07:29:34'))}`);
  const DEG = 'STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:DEGRADADO,ESTADO:R1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:07:29:30';
  const AUTO = 'STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:07:31:00';

  const montar = () => {
    const a = montarAppLimpia();
    a.meter = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
    a.eventos = () => Array.from(a.d.querySelectorAll('#event-feed .event-item'))
                           .map(n => n.textContent.replace(/\s+/g, ' '));
    a.cartel = () => a.d.getElementById('aviso-radio-deg');
    a.txt = () => (a.cartel() ? a.cartel().textContent.replace(/\s+/g, ' ') : '');
    a.visible = () => !!a.cartel() && !a.cartel().hidden;
    return a;
  };

  // (1) Primera trama: cartel visible con el texto pedido y la hora; una linea y un toast.
  const a = montar();
  a.meter(DEG);
  const antes = a.eventos().length;
  a.meter(VUELTA('07:29:34'));
  const nuevas = a.eventos().slice(0, a.eventos().length - antes);
  assert(a.visible() && /LA RADIO VOLVIO entre los dos postes\. Siguen en Degradado\./.test(a.txt()) &&
         /VOLVER AL MENU en el POSTE 1 \(el Poste 2 lo sigue solo por radio\)/.test(a.txt()) &&
         /07:29:34/.test(a.txt()),
    `RadioDeg: sale el cartel visible con el texto y la hora del aviso: "${a.txt().slice(0, 200)}"`);
  assert(nuevas.length === 1 && /ENLACE_DISPONIBLE/.test(nuevas[0]) && /LA RADIO VOLVIO/.test(nuevas[0]),
    `RadioDeg: la primera trama escribe UNA linea con el literal y la traduccion: ${JSON.stringify(nuevas)}`);
  const toast = a.d.getElementById('toast-msg');
  assert(toast && /RADIO VOLVIO/i.test(toast.textContent), `RadioDeg: la primera trama saca toast: "${toast && toast.textContent}"`);

  // (2) La repeticion de 60 s: sin linea ni toast nuevos, pero el cartel pasa a la hora nueva.
  toast.textContent = '';
  const n1 = a.eventos().length;
  a.meter(VUELTA('07:30:34'));
  assert(a.eventos().length === n1 && toast.textContent === '',
    `RadioDeg: la repeticion no escribe linea ni toast (lineas ${n1}->${a.eventos().length}, toast "${toast.textContent}")`);
  assert(a.visible() && /07:30:34/.test(a.txt()) && !/07:29:34/.test(a.txt()),
    `RadioDeg: el cartel lleva la hora del ULTIMO aviso: "${a.txt().slice(0, 200)}"`);

  // (3) Un $STATUS que sigue en DEGRADADO no lo quita; uno que ya no lo dice, si.
  a.meter(DEG);
  assert(a.visible(), 'RadioDeg: un $STATUS con MODO:DEGRADADO deja el cartel');
  a.meter(AUTO);
  assert(!a.visible(), 'RadioDeg: un $STATUS sin MODO:DEGRADADO oculta el cartel');

  // (4) Entendido lo cierra, y la repeticion en la misma conexion no lo vuelve a abrir.
  const b = montar();
  b.meter(DEG);
  b.meter(VUELTA('07:29:34'));
  const ok = b.d.getElementById('btn-aviso-radio-deg-ok');
  if (ok) ok.click();
  assert(!b.visible(), 'RadioDeg: el boton Entendido cierra el cartel');
  const n2 = b.eventos().length;
  b.meter(VUELTA('07:30:34'));
  assert(!b.visible() && b.eventos().length === n2,
    'RadioDeg: tras Entendido, la repeticion de 60 s no reabre el cartel ni escribe linea');

  // (5) Se rearma al reconectar: cartel, linea y toast otra vez.
  b.d.getElementById('btn-bt-disconnect').click();
  assert(!b.visible(), 'RadioDeg: al soltar el enlace el cartel se retira');
  b.d.getElementById('btnDevice').click();
  b.d.querySelector('.bt-device-item').click();
  b.meter(DEG);
  const n3 = b.eventos().length;
  b.meter(VUELTA('07:31:34'));
  assert(b.visible() && b.eventos().length === n3 + 1 && /07:31:34/.test(b.txt()),
    `RadioDeg: tras reconectar vuelve el cartel y UNA linea (lineas ${n3}->${b.eventos().length})`);
};
