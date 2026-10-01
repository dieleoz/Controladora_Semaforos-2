// ===== tests/dom_deg_auto.js =====
// DEGRADADO AUTOMATICO (A-15), tarjeta de js/deg_auto.js. Las tramas son las del
// firmware de a0d605b, copiadas de Maestro/src/bluetooth.cpp y deg_auto.cpp:
//   sale    CMD:PIN:1234:SET_DEG_AUTO:1|0   y   CMD:CONSULTA_DEG_AUTO (sin PIN)
//   entra   $ACK,CMD:SET_DEG_AUTO,RESULT:...  $ERR,CMD:SET_DEG_AUTO,DESC:...
//           $ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_x_OTRO_x_APTO_x
//           $EVENT,NODE:..,ORIGEN:DEGRADADO,DETALLE:..,HORA:..   (bluetooth_reportarEvento)
//           $ALARM,NODE:..,EVENTO:DEGRADADO,CAUSA:..,<tramo>,ACCION:..,HORA:.. (reportarAlarma)
// Lo llama test_dom_execution.js, como dom_version.js.
module.exports = async function pruebaDegAuto(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const d = a.d;
  const entra = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const txt = (id) => (d.getElementById(id) || {}).textContent || '';
  const ultimo = () => { const e = d.querySelector('#event-feed .event-item'); return e ? e.textContent : ''; };
  const on = d.getElementById('btn-degauto-on');
  const off = d.getElementById('btn-degauto-off');

  // (1) Al identificar el poste la app consulta sola, SIN PIN, y hasta que contesta no pinta un estado.
  // TODO SINCRONO, A PROPOSITO: montarAppLimpia() dispara DOMContentLoaded a mano y jsdom lo
  // vuelve a disparar al vaciarse la cola; tras un `await` hay una segunda app sin enlace
  // debajo de la primera. El temporizador de la consulta se captura en vez de esperarlo.
  a.w.clearTimeout(a.w.DegAuto._timerConsulta);
  const relojReal = a.w.setTimeout, armados = [];
  a.w.setTimeout = (fn, ms) => { armados.push({ fn, ms: ms || 0 }); return 0; };
  entra('STATUS,NODE:ESCLAVO,SERIE:SEM-E-01,MODO:AUTO,ESTADO:R1_V2,T:28,RF:95,RTT:75,BAT:12.8,HORA:14:32:00');
  a.w.setTimeout = relojReal;
  armados.filter(t => t.ms === 0).forEach(t => t.fn());
  const consulta = armados.filter(t => t.ms === a.w.DegAuto.CONSULTA_TRAS_MS);
  assert(txt('degauto-este') === 'consultando...' && a.tramas.length === 0 && consulta.length === 1,
    `DegAuto: al identificar un poste se arma UNA consulta y la tarjeta dice "consultando...", no un ON/OFF: "${txt('degauto-este')}" armadas=${consulta.length}`);
  consulta.forEach(t => t.fn());
  assert(a.tramas.length === 1 && a.tramas[0] === 'CMD:CONSULTA_DEG_AUTO\r\n',
    `DegAuto: al vencer sale UNA CMD:CONSULTA_DEG_AUTO sin PIN: ${a.tramas.join(' | ')}`);
  entra('ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_OFF_OTRO_ON_APTO_NO');
  assert(txt('degauto-este') === 'OFF' && /^ON /.test(txt('degauto-otro')) && txt('degauto-apto') === 'NO',
    `DegAuto: la consulta pinta este=OFF, otro=ON, apto=NO: ${txt('degauto-este')} / ${txt('degauto-otro')} / ${txt('degauto-apto')}`);

  // (2) Activar pide PIN y no escribe nada hasta teclearlo; luego la trama EXACTA.
  a.tramas.length = 0;
  on.click();
  assert(d.getElementById('pin-modal').classList.contains('active') && a.tramas.length === 0,
    `DegAuto: Activar sin PIN abre el teclado y no sale nada: ${a.tramas.join(' | ')}`);
  ['1', '2', '3', '4'].forEach(k => d.querySelector(`.pin-btn[data-key="${k}"]`).click());
  assert(a.tramas.length === 1 && a.tramas[0] === 'CMD:PIN:1234:SET_DEG_AUTO:1\r\n',
    `DegAuto: tecleado el PIN sale CMD:PIN:1234:SET_DEG_AUTO:1 y solo eso: ${a.tramas.join(' | ')}`);
  assert(on.disabled && off.disabled && /Esperando al otro poste/.test(on.textContent),
    `DegAuto: mientras falta el $ACK diferido los botones esperan: "${on.textContent}" disabled=${on.disabled}`);
  assert(txt('degauto-este') === 'OFF',
    `DegAuto: pulsar NO pinta ON; lo pinta la consulta: "${txt('degauto-este')}"`);

  // (3) Cada acuse da su texto, suelta los botones y vuelve a consultar.
  const acuses = [
    ['ON_EFECTIVO', '1', /ACTIVO y el otro poste tambien/],
    ['ON_FALTA_EL_OTRO_POSTE', '1', /activado en ESTE poste.*asi NO hace nada/],
    ['OFF', '0', /DESACTIVADO en este poste/],
  ];
  for (const [res, v, re] of acuses) {
    // La primera vuelta contesta a la orden del PIN de (2), que sigue esperando.
    if (!on.disabled) { a.tramas.length = 0; (v === '1' ? on : off).click(); }
    const salio = a.tramas.slice();
    a.tramas.length = 0;
    entra('ACK,CMD:SET_DEG_AUTO,RESULT:' + res);
    assert(salio.length === 1 && salio[0] === 'CMD:PIN:1234:SET_DEG_AUTO:' + v + '\r\n' && re.test(ultimo()),
      `DegAuto: $ACK RESULT:${res} tras SET_DEG_AUTO:${v} pinta su texto: [${salio.join('|')}] "${ultimo().slice(0, 120)}"`);
    assert(!on.disabled && on.textContent === 'Activar' && off.textContent === 'Desactivar' &&
           a.tramas.length === 1 && a.tramas[0] === 'CMD:CONSULTA_DEG_AUTO\r\n',
      `DegAuto: tras RESULT:${res} se sueltan los botones y se vuelve a consultar: ${a.tramas.join(' | ')}`);
  }
  entra('ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_ON_OTRO_ON_APTO_SI');
  assert(txt('degauto-este') === 'ON' && /^ON /.test(txt('degauto-otro')) && txt('degauto-apto') === 'SI',
    `DegAuto: la consulta ESTE_ON_OTRO_ON_APTO_SI pinta ON / ON / SI: ${txt('degauto-este')} / ${txt('degauto-otro')} / ${txt('degauto-apto')}`);

  // (4) Cada rechazo da su texto y NO cambia el estado pintado.
  const rechazos = [
    ['FORMATO_INVALIDO', /no entendio la orden/],
    ['SIN_ENLACE_CON_EL_OTRO_POSTE', /No hay radio con el otro poste/],
    ['EN_DEGRADADO_SALGA_PRIMERO', /Este poste esta en degradado: saquelo primero/],
    ['CAMBIADO_AQUI_SIN_ACUSE_DEL_OTRO_POSTE', /Cambiado en ESTE poste, pero el otro no lo ha acusado/],
  ];
  for (const [desc, re] of rechazos) {
    on.click();
    entra('ERR,CMD:SET_DEG_AUTO,DESC:' + desc);
    assert(re.test(ultimo()) && !on.disabled && txt('degauto-este') === 'ON',
      `DegAuto: $ERR DESC:${desc} pinta su texto y suelta el boton: "${ultimo().slice(0, 140)}"`);
  }

  // (5) Eventos y alarmas del degradado, con la forma exacta de bluetooth_reportarEvento/Alarma.
  entra('EVENT,NODE:MAESTRO,ORIGEN:DEGRADADO,DETALLE:AUTO_ENTRADA_INICIO_14:40:00,HORA:14:35:00');
  assert(/rojo hasta 14:40/.test(ultimo()) && /rojo hasta 14:40/.test(txt('toast-msg')),
    `DegAuto: $EVENT AUTO_ENTRADA_INICIO_14:40:00 dice "rojo hasta 14:40": "${ultimo().slice(0, 160)}"`);
  entra('EVENT,NODE:ESCLAVO,ORIGEN:DEGRADADO,DETALLE:ENLACE_DISPONIBLE,HORA:14:50:00');
  assert(/LA RADIO VOLVIO entre los dos postes\. Siguen en Degradado/.test(ultimo()),
    `DegAuto: $EVENT ENLACE_DISPONIBLE dice que sigue en degradado: "${ultimo().slice(0, 160)}"`);
  const alarmas = [
    ['MAESTRO', 'AUTO_NO_HORA', 'RF:97%,RTT:70ms,SINRESP:0', 'SIGUE_AMBAR',
     /No pudo entrar solo: reloj sin poner en hora.*\. Sigue en ambar/],
    ['MAESTRO', 'AUTO_NO_INICIO', 'RF:97%,RTT:70ms,SINRESP:0', 'SIGUE_AMBAR',
     /No pudo entrar solo: la hora de arranque ya habia pasado/],
    // Maestro deg_auto.cpp: MDT_NO_GUARDADO -> ACCION:QUEDA_ROJO; no puede decir ambar.
    ['MAESTRO', 'AUTO_NO_GUARDADO', 'RF:97%,RTT:70ms,SINRESP:0', 'QUEDA_ROJO',
     /No pudo entrar solo: no se pudo guardar el testigo.*ROJO FIJO(?!.*Sigue en ambar)/],
    ['ESCLAVO', 'AUTO_NO_DESFASE', 'RX:10,OK:9,RUIDO:1', 'SIGUE_AMBAR',
     /No pudo entrar solo: la hora del poste no cuadra/],
    ['MAESTRO', 'OTRO_EN_DEGRADADO', 'RF:--,RTT:--,SINRESP:3', 'REVISE_OTRO',
     /El otro poste esta en degradado y este no/],
    ['ESCLAVO', 'RENOVAR_TESTIGO', 'RX:10,OK:9,RUIDO:1', 'REPITA_TESTIGO',
     /28 dias o mas sin renovar el testigo/],
  ];
  for (const [nodo, causa, tramo, accion, re] of alarmas) {
    entra(`ALARM,NODE:${nodo},EVENTO:DEGRADADO,CAUSA:${causa},${tramo},ACCION:${accion},HORA:14:36:00`);
    assert(re.test(ultimo()),
      `DegAuto: $ALARM CAUSA:${causa} da su texto: "${ultimo().slice(0, 160)}"`);
  }

  // (6) El testigo ya no vence (29/09): ni la tarjeta ni el aviso de renovar dicen 31 dias.
  const tarjeta = (d.getElementById('card-testigo') || {}).textContent || '';
  const renovar = ultimo();
  const vence31 = /31\s*d[ií]as|vence|caduca/i;
  assert(tarjeta.length > 0 && /testigo/.test(tarjeta) && !vence31.test(tarjeta),
    `DegAuto: la tarjeta del testigo NO dice que vence a 31 dias: "${(tarjeta.match(vence31) || [''])[0]}"`);
  assert(/renovar/.test(renovar) && !vence31.test(renovar),
    `DegAuto: el aviso RENOVAR_TESTIGO pide renovar y no dice que venza: "${renovar.slice(0, 160)}"`);

  // (5b) Todo codigo que puede llegar en CAUSA:AUTO_NO_<codigo> se traduce: censo de
  // nombreMotivo() (Maestro/src/deg_auto.cpp) y nombreRechazo() (Esclavo/src/deg_auto.cpp).
  // OK es su default: sale si modo_degradado_entrarTestigo() devuelve MDT_RENOVADO.
  const motivos = [
    ['MAESTRO', 'HORA'], ['MAESTRO', 'DESFASE'], ['MAESTRO', 'DESPEJE'], ['MAESTRO', 'INICIO'],
    ['MAESTRO', 'AMBAR'], ['MAESTRO', 'EN_VERDE'], ['MAESTRO', 'GUARDADO'], ['MAESTRO', 'OK'],
    ['ESCLAVO', 'HORA'], ['ESCLAVO', 'DESFASE'], ['ESCLAVO', 'INICIO'], ['ESCLAVO', 'DESPEJE'],
    ['ESCLAVO', 'AMBAR'], ['ESCLAVO', 'EN_VERDE'], ['ESCLAVO', 'GUARDADO'],
  ];
  for (const [nodo, nombre] of motivos) {
    entra(`ALARM,NODE:${nodo},EVENTO:DEGRADADO,CAUSA:AUTO_NO_${nombre},RF:97%,RTT:70ms,SINRESP:0,` +
          'ACCION:SIGUE_AMBAR,HORA:14:37:00');
    const crudo = 'solo: ' + nombre + '.';   // asi lo sacaria el fallback de _motivo()
    const t = ultimo().replace(/^[\s\S]*?(No pudo entrar solo: )/, '$1');  // la Caja Negra repite la CAUSA cruda
    assert(/No pudo entrar solo: /.test(t) && t.indexOf(crudo) < 0 && t.indexOf('sin motivo') < 0,
      `DegAuto: AUTO_NO_${nombre} se traduce y no sale "${crudo}" en crudo: "${t.slice(0, 160)}"`);
  }
};
