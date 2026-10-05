// ===== tests/dom_aviso_degradado.js =====
// AVISO DE PALETEROS (js/aviso_degradado.js, decision del 29/09). Con el poste en
// Degradado ($STATUS MODO:DEGRADADO) cambiar el modo de UN poste abre un aviso y NO
// escribe nada hasta confirmarlo; confirmar deja salir la orden UNA vez. El rojo de
// emergencia y cualquier orden fuera de Degradado no preguntan.
// Lo llama test_dom_execution.js, como dom_version.js.
module.exports = async function pruebaAvisoDegradado(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const d = a.d;
  a.w.clearTimeout(a.w.DegAuto._timerConsulta);   // la consulta automatica no cuenta aqui
  const entra = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const modo = (m, rf = 'RF:0', esc = '') =>
    entra(`STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:${m},ESTADO:R1_R2,T:31,${rf},RTT:0,BAT:12.9,HORA:14:31:00${esc}`);
  const opcion = (v) => entra(`ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_${v}_OTRO_ON_APTO_SI`);
  const modal = d.getElementById('aviso-deg-modal');
  const chk = d.getElementById('chk-aviso-deg');
  const ok = d.getElementById('btn-aviso-deg-confirmar');
  const manual = d.querySelector('[data-cmd="SET_MODO:MANUAL"]');
  const ultimo = () => { const e = d.querySelector('#event-feed .event-item'); return e ? e.textContent : ''; };
  const MANUAL = 'CMD:PIN:1234:SET_MODO:MANUAL\r\n';

  modo('DEGRADADO');
  // (1) En Degradado, SET_MODO:MANUAL (PIN incluido) abre el aviso y no sale ninguna trama.
  a.tramas.length = 0;
  manual.click();
  ['1', '2', '3', '4'].forEach(k => d.querySelector(`.pin-btn[data-key="${k}"]`).click());
  assert(modal.classList.contains('active') && a.tramas.length === 0,
    `Aviso: con MODO:DEGRADADO, SET_MODO:MANUAL abre el aviso y NO sale nada: ${a.tramas.join(' | ')}`);
  assert(/SET_MODO:MANUAL/.test(d.getElementById('aviso-deg-orden').textContent) && ok.disabled,
    `Aviso: el aviso nombra la orden y "Cambiar este poste" esta apagado sin la casilla`);

  // (2) Cancelar no envia.
  d.getElementById('btn-aviso-deg-cancelar').click();
  assert(!modal.classList.contains('active') && a.tramas.length === 0 && /cancelada.*no se envio nada/.test(ultimo()),
    `Aviso: Cancelar cierra y no envia: [${a.tramas.join(' | ')}] "${ultimo().slice(0, 100)}"`);

  // (3) Sin la casilla, confirmar no hace nada; con la casilla envia UNA vez, y el vale no se reusa.
  manual.click();
  ok.click();
  assert(modal.classList.contains('active') && a.tramas.length === 0,
    `Aviso: sin marcar la casilla, confirmar no envia: ${a.tramas.join(' | ')}`);
  // La segunda barrera: con el boton encendido a la fuerza y la casilla sin marcar, tampoco.
  ok.disabled = false;
  ok.click();
  assert(modal.classList.contains('active') && a.tramas.length === 0,
    `Aviso: _confirmar() mira la casilla y no solo el boton apagado: ${a.tramas.join(' | ')}`);
  chk.checked = true;
  chk.dispatchEvent(new a.w.Event('change'));
  ok.click();
  assert(!modal.classList.contains('active') && a.tramas.length === 1 && a.tramas[0] === MANUAL,
    `Aviso: casilla marcada y confirmar envia SET_MODO:MANUAL UNA vez: ${a.tramas.join(' | ')}`);
  manual.click();
  assert(modal.classList.contains('active') && a.tramas.length === 1,
    `Aviso: el vale es de un solo uso; la siguiente pulsacion vuelve a preguntar: ${a.tramas.join(' | ')}`);
  d.getElementById('btn-aviso-deg-cancelar').click();

  // (4) Desactivar el degradado automatico tambien cambia el modo: pregunta igual. Con RF medido:
  // sin radio la eleccion ni se ofrece (encargo C, lo prueba dom_deg_eleccion.js).
  modo('DEGRADADO', 'RF:97');
  a.tramas.length = 0;
  d.getElementById('btn-degauto-off').click();
  assert(modal.classList.contains('active') && a.tramas.length === 0,
    `Aviso: en Degradado, SET_DEG_AUTO:0 abre el aviso y no sale nada: ${a.tramas.join(' | ')}`);
  d.getElementById('btn-aviso-deg-cancelar').click();

  // (5) FORZAR_ROJO no pregunta, ni en Degradado.
  a.tramas.length = 0;
  d.getElementById('btn-op-emergency').click();
  assert(!modal.classList.contains('active') && a.tramas.length === 1 && a.tramas[0] === 'CMD:FORZAR_ROJO\r\n',
    `Aviso: FORZAR_ROJO sale sin preguntar en Degradado: ${a.tramas.join(' | ')}`);

  // (6) Sin Degradado y con la opcion automatica APAGADA (acuse) no pregunta.
  opcion('OFF');
  modo('AUTO');
  a.tramas.length = 0;
  manual.click();
  assert(!modal.classList.contains('active') && a.tramas.length === 1 && a.tramas[0] === MANUAL,
    `Aviso: con MODO:AUTO, SET_MODO:MANUAL sale sin aviso: ${a.tramas.join(' | ')}`);

  // (7) Riesgo (f).1 de SPEC_2 7.ter: opcion ON por acuse, sin radio (ESC:? y RF:0), MODO no
  // DEGRADADO. El otro poste puede entrar solo a los 300 s: pregunta igual, y no sale nada.
  opcion('ON');
  modo('AUTO', 'RF:0', ',ESC:?');
  a.tramas.length = 0;
  manual.click();
  const cuerpo = d.getElementById('aviso-deg-cuerpo');
  assert(modal.classList.contains('active') && a.tramas.length === 0 &&
         /degradado automatico activado/.test(cuerpo ? cuerpo.textContent : ''),
    `Aviso: opcion ON y sin radio, SET_MODO:MANUAL abre el aviso del automatico y NO sale nada: ` +
    `[${a.tramas.join(' | ')}] "${cuerpo ? cuerpo.textContent.slice(0, 60) : 'sin #aviso-deg-cuerpo'}"`);
  d.getElementById('btn-aviso-deg-cancelar').click();

  // (8) La misma opcion ON CON radio (ESC:ROJO, RF:95) no pregunta.
  modo('AUTO', 'RF:95', ',ESC:ROJO');
  a.tramas.length = 0;
  manual.click();
  assert(!modal.classList.contains('active') && a.tramas.length === 1 && a.tramas[0] === MANUAL,
    `Aviso: opcion ON con radio, SET_MODO:MANUAL sale sin aviso: ${a.tramas.join(' | ')}`);
};
