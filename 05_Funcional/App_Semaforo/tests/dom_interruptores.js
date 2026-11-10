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

  // N-187 (SPEC_8 8.6; P3 del responsable 09/11: R en 3..5 s, fabrica 3). Datos asimetricos (C NC, D NA, VE C0D1)
  // para que un indice cruzado no pase.
  const det = d.getElementById('instalacion'), tx = (id) => (d.getElementById(id) || {}).textContent || '';
  assert(!!det && !!det.closest('#tab-diag.admin-only') && det.previousElementSibling === d.getElementById('averias'),
    'J1: "Instalacion: camaras y barrera" vive en Ajustes tras la clave (D-56), justo despues de Averias');
  if (det) {
    a.tramas.length = 0;
    det.open = true; det.dispatchEvent(new w.Event('toggle')); det.dispatchEvent(new w.Event('toggle'));
    assert(a.tramas.join('|') === 'CMD:CONSULTA_INSTALACION\r\n',
      `J1: al abrir sale UNA consulta y sin PIN: ${JSON.stringify(a.tramas)}`);
    entra('ACK,CMD:CONSULTA_INSTALACION,RESULT:C_NC_D_NA_R4,VE:C0D1');
    assert(tx('inst-c') === 'NC' && tx('inst-d') === 'NA' && tx('inst-r') === '4 s' && /C 0 . D 1/.test(tx('inst-ve')) &&
           /Qu[eé] falta:.*c[aá]mara D/.test(tx('inst-falta')) && /nadie delante debe salir 0/i.test(tx('inst-ve-ayuda')) &&
           /contacto mal declarado o algo delante/.test(tx('inst-ve-ayuda')) && /NC/.test(tx('inst-c-ayuda')),
      `J2: la consulta pinta C NC, D NA, 4 s y VE con su ayuda: ${['c', 'd', 'r', 've', 'falta'].map(x => tx('inst-' + x)).join(' / ')}`);
    const ops = Array.from(d.querySelectorAll('#inst-retardo option')).map(o => o.value).join(',');
    assert(ops === '3,4,5' && /3 a 5 s/.test(tx('inst-r-ayuda')) && /ROJO/.test(tx('inst-r-ayuda')),
      `J3: el retardo se elige en 3..5 (P3), con su ayuda: [${ops}] "${tx('inst-r-ayuda')}"`);
    a.tramas.length = 0;
    d.querySelector('[data-instalacion="SET_CAMARA:C:NA"]').click(); pin();
    d.getElementById('inst-retardo').value = '5'; d.getElementById('btn-inst-retardo').click();
    assert(a.tramas.join('|') === 'CMD:PIN:1234:SET_CAMARA:C:NA\r\n|CMD:PIN:1234:SET_PLUMA_RETARDO:5\r\n',
      `J3: los mandos salen con PIN y el valor exacto: ${JSON.stringify(a.tramas)}`);
    a.tramas.length = 0;
    entra('ACK,CMD:SET_CAMARA:C:NA,RESULT:OK,VE:C0D0');
    assert(/guardado y rele[ií]do/.test(ultimo()) && /C 0, c[aá]mara D 0/.test(ultimo()) && !/ACEPTADA \(/.test(ultimo()) &&
           a.tramas.join('|') === 'CMD:CONSULTA_INSTALACION\r\n' && !JERGA.test(toast()),
      `J4: OK dice guardado, trae VE y reconsulta: "${ultimo().slice(0, 160)}" ${JSON.stringify(a.tramas)}`);
    entra('ACK,CMD:SET_PLUMA_RETARDO:4,RESULT:SIN_CAMBIO,VE:C1D0');
    assert(/ya estaba/.test(ultimo()) && /retardo/.test(ultimo()) && /c[aá]mara C 1/.test(ultimo()),
      `J4: SIN_CAMBIO (con argumento) dice ya estaba y trae VE: "${ultimo().slice(0, 160)}"`);
    const J5 = [['ERR,CMD:SET_PLUMA_RETARDO:2,DESC:FUERA_DE_RANGO', /3 a 5 s/], ['ERR,CMD:SET_PLUMA_RETARDO:6,DESC:FUERA_DE_RANGO', /3 a 5 s/],
      ['ERR,CMD:SET_CAMARA,DESC:FORMATO_INVALIDO', /no entendi/], ['ERR,CMD:SET_CAMARA:C:NC,DESC:NO_QUEDO_PUESTO', /NO lo dej/],
      ['ERR,CMD:SET_CAMARA:D:NC,DESC:PLUMA_VETADA', /Sacar las c[aá]maras del veto.*presencia da 0.*Devolver el veto/]];
    J5.forEach(([carga, si]) => {
      entra(carga);
      assert(si.test(ultimo()) && /Qu[eé] falta/.test(ultimo()) && !/sin traducir/.test(ultimo()) && !JERGA.test(toast()),
        `J5: ${carga} -> "${ultimo().slice(0, 220)}" / "${toast()}"`);
    });
    // J6: firmware sin N-187. La consulta vuelve AUTH_FAILED (Esclavo) o DESCONOCIDO (Maestro): el panel lo dice.
    [['ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO', 'Esclavo'], ['ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO', 'Maestro']]
      .forEach(([carga, quien]) => {
        a.tramas.length = 0;
        d.getElementById('btn-inst-consultar').click();
        entra(carga);
        assert(a.tramas.join('|') === 'CMD:CONSULTA_INSTALACION\r\n' && /no tiene la instalaci[oó]n de c[aá]maras y barrera/
          .test(tx('inst-falta')) && /Qu[eé] falta: cargar el firmware nuevo en este poste/.test(ultimo()),
          `J6: ${quien} viejo (${carga}) -> panel "${tx('inst-falta')}" / evento "${ultimo().slice(0, 160)}"`);
      });
    entra('ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO');   // sin consulta en vuelo no es de la instalacion
    assert(!/instalaci[oó]n de c[aá]maras/.test(ultimo()),
      `J6: control: un DESCONOCIDO sin consulta en vuelo no se atribuye a la instalacion: "${ultimo().slice(0, 120)}"`);
  }
};
