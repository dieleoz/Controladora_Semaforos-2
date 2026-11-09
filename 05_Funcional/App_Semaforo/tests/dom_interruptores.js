// ===== tests/dom_interruptores.js =====
// SPEC_8 6 en la APP (D-94, D-97, D-148, D-151, D-152; D-56 la clave, D-80 sin jerga, D-83 ambar = mirar).
// Los textos esperados salen de SPEC_8 6.1 y 6.4; las tramas, del firmware: interruptores_orden() de
// {Maestro,Esclavo}/src/interruptores.cpp (responder(): "$ERR,CMD:<orden>:<valor>,DESC:..."; sin valor solo
// FORMATO_INVALIDO) e interruptores_avisoArranque() para el $EVENT de D-152. Los $ACK se ejercen tambien sin
// argumento: el firmware lo manda siempre, y esa forma ejerce el _pedido de js/interruptores.js.
// Lo llama test_dom_execution.js, como dom_version.js.
module.exports = async function pruebaInterruptores(montarAppLimpia, assert) {
  const JERGA = require('./dom_usabilidad.js').JERGA;
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const d = a.d, w = a.w;
  if (w.DegAuto && w.DegAuto._timerConsulta) w.clearTimeout(w.DegAuto._timerConsulta);
  const entra = (carga) => w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const ultimo = () => { const e = d.querySelector('#event-feed .event-item'); return e ? e.textContent : ''; };
  const toast = () => d.getElementById('toast-msg').textContent;
  const pin = () => ['1', '2', '3', '4'].forEach(k => d.querySelector(`.pin-btn[data-key="${k}"]`).click());
  const dialogo = () => d.getElementById('dialogo-modal');
  const abierto = () => dialogo().classList.contains('active');
  const aceptar = () => d.getElementById('btn-dialogo-aceptar').click();
  const cancelar = () => d.getElementById('btn-dialogo-cancelar').click();
  const boton = (orden) => d.querySelector(`[data-interruptor="${orden}"]`);
  const st = (est, extra) => entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:' + est +
    ',T:9,RF:97,RTT:70,BAT:12.9,HORA:14:31:00' + extra);

  // I1 (D-56, D-94): los cuatro mandos existen, viven en Diagnostico y solo se ven tras la clave.
  const ordenes = ['SET_VETO_CAMARA:OFF', 'SET_VETO_CAMARA:ON', 'SET_BARRERA_SERVICIO:OFF', 'SET_BARRERA_SERVICIO:ON'];
  const faltan = ordenes.filter(o => !boton(o));
  assert(faltan.length === 0, `I1: los mandos de SPEC_8 6.1 existen (faltan: ${faltan.join(', ') || 'ninguno'})`);
  if (!faltan.length) {
  assert(ordenes.every(o => boton(o).closest('#tab-diag.admin-only')),
    'I1: los mandos viven en Diagnostico (#tab-diag, admin-only): solo tras la clave de administracion (D-56)');

  // I2 (SPEC_8 6.1): antes de mandar OFF de la barrera, la consecuencia; cancelar no envia nada.
  a.tramas.length = 0;
  boton('SET_BARRERA_SERVICIO:OFF').click();
  const txtB = d.getElementById('dialogo-texto').textContent;
  assert(abierto() && /arriba/i.test(txtB) && /rojo/i.test(txtB) && /AHORA/.test(txtB) && /servicio/i.test(txtB) &&
         !JERGA.test(txtB),
    `I2: OFF de la barrera pregunta antes: sube AHORA, arriba tambien en rojo, hasta ponerla en servicio: "${txtB}"`);
  cancelar();
  assert(a.tramas.length === 0, `I2: cancelar no envia nada (${a.tramas.length} tramas)`);

  // I3: aceptar con PIN pendiente pide el PIN y, tecleado, sale la orden exacta con PIN (D-94).
  boton('SET_BARRERA_SERVICIO:OFF').click(); aceptar(); pin();
  const tB = a.tramas.filter(t => /SET_BARRERA_SERVICIO/.test(t));
  assert(tB.length === 1 && tB[0] === 'CMD:PIN:1234:SET_BARRERA_SERVICIO:OFF\r\n',
    `I3: sale UNA orden CMD:PIN:1234:SET_BARRERA_SERVICIO:OFF: ${JSON.stringify(tB)}`);

  // I4: OFF de la camara dice tambien su consecuencia (tabla de SPEC_8 6), y sale tras aceptar.
  a.tramas.length = 0;
  boton('SET_VETO_CAMARA:OFF').click();
  const txtC = d.getElementById('dialogo-texto').textContent;
  assert(abierto() && /dos c[aá]maras/i.test(txtC) && /sube con el verde/i.test(txtC) && /retardo/i.test(txtC) &&
         !JERGA.test(txtC),
    `I4: OFF de la camara dice la consecuencia de la tabla de SPEC_8 6: "${txtC}"`);
  aceptar();
  assert(a.tramas.length === 1 && a.tramas[0] === 'CMD:PIN:1234:SET_VETO_CAMARA:OFF\r\n',
    `I4: tras aceptar sale SET_VETO_CAMARA:OFF con PIN: ${JSON.stringify(a.tramas)}`);

  // I5: ON es el lado seguro: sale sin preguntar.
  a.tramas.length = 0;
  boton('SET_VETO_CAMARA:ON').click();
  assert(!abierto() && a.tramas.length === 1 && a.tramas[0] === 'CMD:PIN:1234:SET_VETO_CAMARA:ON\r\n',
    `I5: ON sale sin dialogo: ${JSON.stringify(a.tramas)}`);
  }

  // I6 (D-94, molde SET_TIEMPOS): cada RESULT con su frase, nunca el generico "ACEPTADA (".
  const ACKS = [
    ['ACK,CMD:SET_VETO_CAMARA,RESULT:OK', /veto/i, /ACEPTADA \(/],
    ['ACK,CMD:SET_VETO_CAMARA,RESULT:SIN_CAMBIO', /ya estaba/i, /ACEPTADA \(/],
    ['ACK,CMD:SET_BARRERA_SERVICIO:ON,RESULT:OK', /en servicio/i, /fuera de servicio/i],
    ['ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:OK', /fuera de servicio/i, /ACEPTADA \(/],
    ['ACK,CMD:SET_BARRERA_SERVICIO,RESULT:SIN_CAMBIO', /ya estaba/i, /ACEPTADA \(/]
  ];
  ACKS.forEach(([carga, si, no]) => {
    entra(carga);
    assert(si.test(ultimo()) && !no.test(ultimo()) && !/ACEPTADA \(/.test(ultimo()) && !JERGA.test(toast()),
      `I6: ${carga} -> "${ultimo().slice(0, 180)}" / toast "${toast()}"`);
  });

  // I7 (D-148): cada $ERR con su frase llana; RESPALDO_INVALIDO dice pila y reiniciar.
  const ERRS = [
    ['ERR,CMD:SET_BARRERA_SERVICIO:OFF,DESC:RESPALDO_INVALIDO', /pila/i],
    ['ERR,CMD:SET_VETO_CAMARA:OFF,DESC:RESPALDO_INVALIDO', /pila/i],
    ['ERR,CMD:SET_VETO_CAMARA,DESC:FORMATO_INVALIDO', /no se ejecut/i],
    // SPEC_8 6.1 (condicion del arquitecto): lo releido tras escribir no es lo pedido. No es el
    // NO_QUEDO_PUESTA del reloj (js/textos_err.js).
    ['ERR,CMD:SET_BARRERA_SERVICIO:OFF,DESC:NO_QUEDO_PUESTO', /NO lo dej[oó] puesto/],
    ['ERR,CMD:SET_VETO_CAMARA:ON,DESC:NO_QUEDO_PUESTO', /NO lo dej[oó] puesto/]
  ];
  ERRS.forEach(([carga, si]) => {
    entra(carga);
    assert(si.test(ultimo()) && !/sin traducir/i.test(ultimo()) && !JERGA.test(toast()),
      `I7: ${carga} -> "${ultimo().slice(0, 200)}" / toast "${toast()}"`);
  });
  entra('ERR,CMD:SET_BARRERA_SERVICIO:OFF,DESC:RESPALDO_INVALIDO');
  assert(/reinici/i.test(ultimo()), `I7: RESPALDO_INVALIDO manda reiniciar con la pila buena: "${ultimo()}"`);

  // I8 (D-97, D-83, D-80): PLUMA:FUERA en ambar con su frase, tambien con la luz en VERDE.
  const pluma = d.getElementById('pluma-estado');
  for (const est of ['ROJO', 'VERDE']) {
    st(est, ',ESC:ROJO,PLUMA:FUERA');
    assert(/FUERA DE SERVICIO/i.test(pluma.textContent) && !/NO RECONOCIDA/.test(pluma.textContent) &&
           /amber/.test(pluma.style.color) && !JERGA.test(pluma.textContent),
      `I8: PLUMA:FUERA con ${est} en ambar y sin jerga: "${pluma.textContent}" color ${pluma.style.color}`);
  }
  st('ROJO', ',ESC:ROJO,PLUMA:ABAJO');
  assert(/PLUMA ABAJO/.test(pluma.textContent) && !/FUERA/.test(pluma.textContent),
    `I8 (control): PLUMA:ABAJO sigue igual: "${pluma.textContent}"`);

  // I9 (D-151): CAM:NOVETO en ambar, "camara sana y sin veto: devolver el veto".
  st('ROJO', ',ESC:ROJO,PLUMA:ABAJO,CAM:NOVETO');
  const camE = d.getElementById('cam-estado'), camD = d.getElementById('cam-detalle');
  assert(camE.textContent !== '!' && /amber/.test(camE.style.color) && /devolver el veto/i.test(camD.textContent) &&
         /sana/i.test(camD.textContent) && !JERGA.test(camE.textContent + ' ' + camD.textContent),
    `I9: CAM:NOVETO -> "${camE.textContent}" / "${camD.textContent}" color ${camE.style.color}`);
  st('ROJO', ',ESC:ROJO,PLUMA:ABAJO,CAM:PEGADA');
  assert(camE.textContent === 'PEGADA', `I9 (control): CAM:PEGADA sigue igual: "${camE.textContent}"`);

  // I10 (D-152): el $EVENT de arranque con una bandera puesta se muestra con frase llana.
  // Literales del firmware (interruptores.cpp, interruptores_avisoArranque), no inventados.
  entra('EVENT,NODE:MAESTRO,ORIGEN:INTERRUPTOR,DETALLE:ARRANQUE_BARRERA_FUERA,HORA:08:00:00');
  assert(/fuera de servicio/i.test(ultimo()) && /arriba/i.test(ultimo()),
    `I10: el aviso de arranque de la barrera fuera se muestra: "${ultimo().slice(0, 200)}"`);
  entra('EVENT,NODE:ESCLAVO,ORIGEN:INTERRUPTOR,DETALLE:ARRANQUE_CAM_NOVETO,HORA:08:00:00');
  assert(/fuera del veto/i.test(ultimo()) && /c[aá]maras/i.test(ultimo()),
    `I10: el aviso de arranque del veto fuera se muestra: "${ultimo().slice(0, 200)}"`);
  entra('EVENT,NODE:MAESTRO,ORIGEN:INTERRUPTOR,DETALLE:ARRANQUE_CAM_NOVETO_Y_BARRERA_FUERA,HORA:08:00:00');
  assert(/fuera del veto/i.test(ultimo()) && /fuera de servicio/i.test(ultimo()),
    `I10: con las dos banderas salen las dos frases: "${ultimo().slice(0, 200)}"`);

  // Control negativo: un $ACK de otra orden sigue por su camino (no lo captura este modulo).
  entra('ACK,CMD:ORDEN_INVENTADA,RESULT:OK');
  assert(/ACEPTADA/.test(ultimo()) && !/veto|servicio/i.test(ultimo()),
    `Control: un $ACK ajeno no se traduce como interruptor: "${ultimo()}"`);
};
