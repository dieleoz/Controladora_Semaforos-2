// ===== tests/dom_claves.js =====
// SPEC_4 9.2 (D-56, D-57): dos claves. La de administracion de la app solo ensena u oculta; el PIN del equipo
// autoriza cada orden. No corre sola: la llama test_dom_execution.js con SU montarAppLimpia y SU assert.
// La clave de soporte NO esta aqui: si existe la variable de entorno CLAVE_SOPORTE se prueba que entra; si no,
// solo que una erronea no entra y que la huella guardada tiene la forma de una FNV-1a de 32 bits.
// Lo que no mide: la caducidad del PIN con el rol puesto (SPEC_4 4), ni el telefono.
module.exports = async function pruebaClaves(montarAppLimpia, assert) {
  const montar = () => {
    const a = montarAppLimpia();
    const el = (id) => a.d.getElementById(id);
    a.el = el;
    a.activo = (id) => !!el(id) && el(id).classList.contains('active');
    a.visible = (id) => !!el(id) && el(id).style.display !== 'none';
    a.clave = (v) => { if (!el('clave-entrada')) return; el('clave-entrada').value = v; el('btn-clave-entrar').click(); };
    a.pulsa = (id) => { if (el(id)) el(id).click(); };
    a.txt = (id) => (el(id) ? el(id).textContent : '');
    a.pin = () => ['1', '2', '3', '4'].forEach(k => a.d.querySelector(`.pin-btn[data-key="${k}"]`).click());
    a.tramas.length = 0;
    return a;
  };
  const PESTANAS = ['tab-btn-tiempos', 'tab-btn-diag', 'tab-btn-depuracion'];

  // 1. Sin clave: Trafico, emergencias y Prueba de focos a la vista; Tiempos, Diagnostico y Depuracion no.
  {
    const a = montar();
    assert(PESTANAS.every(id => !a.visible(id)) && !a.d.body.classList.contains('admin'),
      'Claves: sin clave de administracion no se ven Tiempos, Diagnostico ni Depuracion');
    const focos = a.el('btn-start-test-leds');
    assert(!!focos && !focos.closest('.admin-only') && !!focos.closest('#tab-estado'),
      'Claves: «Prueba de focos» esta en Trafico, fuera de lo que pide la clave de administracion');
    assert(['btn-op-emergency', 'btn-op-ambar-emergencia'].every(id => a.el(id) && !a.el(id).closest('.admin-only')),
      'Claves: ROJO TOTAL y AMBAR EMERGENCIA fuera de lo que pide la clave');
    assert(!!a.el('degauto-pide') && a.el('degauto-pide').classList.contains('solo-admin'),
      'Claves: la tarjeta que pide la eleccion del automatico es solo de administracion');
    a.el('btn-op-emergency').click();
    assert(a.tramas.some(t => /^CMD:FORZAR_ROJO/.test(t)) && !a.activo('pin-modal') && !a.activo('clave-modal'),
      `Claves: FORZAR_ROJO sale sin ninguna clave: ${a.tramas.join(' | ')}`);
    // Prueba de focos sin clave de administracion: pide el PIN, y el PIN no ensena las pestanas.
    a.tramas.length = 0;
    focos.click();
    assert(a.activo('pin-modal') && a.tramas.length === 0, 'Claves: Prueba de focos pide el PIN del equipo');
    a.pin();
    assert(a.tramas.some(t => /^CMD:PIN:1234:TEST_LEDS/.test(t)), `Claves: con el PIN sale: ${a.tramas.join(' | ')}`);
    assert(PESTANAS.every(id => !a.visible(id)), 'Claves: el PIN del equipo NO ensena las pestanas de administracion');
    a.w.DegAutoEleccion.irATestigo();
    assert(/clave de administraci/i.test(a.el('toast-msg').textContent),
      `Claves: ir al testigo sin clave dice que hace falta la de administracion: "${a.el('toast-msg').textContent}"`);
  }

  // 2. El boton de arriba pide la clave de administracion, no el PIN; erronea no entra; 1234 de fabrica si.
  {
    const a = montar();
    a.el('btn-toggle-role').click();
    assert(a.activo('clave-modal') && !a.activo('pin-modal'),
      'Claves: el boton de arriba pide la clave de administracion, no el PIN del equipo');
    a.clave('9999');
    assert(a.activo('clave-modal') && PESTANAS.every(id => !a.visible(id)) && /incorrecta/i.test(a.txt('clave-msg')),
      `Claves: una clave erronea no entra y lo dice: "${a.txt('clave-msg')}"`);
    a.clave('1234');
    assert(!a.activo('clave-modal') && PESTANAS.every(id => a.visible(id)) && a.d.body.classList.contains('admin'),
      'Claves: la de fabrica (1234) ensena Tiempos, Diagnostico y Depuracion');
    // La clave de administracion no autoriza ordenes: Tiempos, y las dos de hora, piden el PIN al pulsar.
    a.el('num-tiempo-verde').value = '5'; a.el('num-tiempo-rojo').value = '5'; a.el('num-tiempo-despeje').value = '30';
    a.el('btn-aplicar-tiempos').click();
    assert(a.tramas.length === 0 && a.activo('pin-modal'),
      `Claves: con la clave de administracion SET_TIEMPOS no sale y abre el PIN: ${a.tramas.join(' | ')}`);
    a.pin(); a.el('btn-aplicar-tiempos').click();   // la orden frenada no se repite sola: se vuelve a pulsar
    assert(a.tramas.some(t => /^CMD:PIN:1234:SET_TIEMPOS:5,5,30/.test(t)), `Claves: con el PIN sale: ${a.tramas.join(' | ')}`);
    const b = montar();
    b.el('btn-toggle-role').click(); b.clave('1234');
    b.pulsa('btn-sync-rtc');
    assert(b.tramas.length === 0 && b.activo('pin-modal'),
      `Claves: sincronizar la hora con la clave de administracion y sin PIN abre el PIN: ${b.tramas.join(' | ')}`);
    b.el('modal-pin-close').click();
    b.el('btn-toggle-role').click();
    assert(PESTANAS.every(id => !b.visible(id)) && !b.d.body.classList.contains('admin'),
      'Claves: el boton de arriba con el rol puesto lo quita');
  }

  // 3. Cambiar la clave: la actual y despues la nueva (4-8 cifras); se guarda la huella, no la clave.
  {
    const a = montar();
    a.el('btn-toggle-role').click();
    a.pulsa('btn-clave-cambiar');
    a.clave('0000');
    assert(/incorrecta/i.test(a.txt('clave-msg')), 'Claves: cambiar exige la clave actual');
    a.clave('1234');
    assert(/nueva/i.test(a.txt('clave-texto')), `Claves: tras la actual pide la nueva: "${a.txt('clave-texto')}"`);
    a.clave('12');
    assert(/4 a 8 cifras/i.test(a.txt('clave-msg')), 'Claves: una nueva de 2 cifras no vale');
    a.clave('123456789');
    assert(/4 a 8 cifras/i.test(a.txt('clave-msg')), 'Claves: ni una de 9');
    a.clave('567890');
    const guardada = a.w.localStorage.getItem('iotvial.clave_admin.v1') || '';
    assert(/^[0-9a-f]{8}$/.test(guardada) && !!a.w.ClaveAdmin && guardada === a.w.ClaveAdmin.huella('567890'),
      `Claves: en localStorage queda la huella de la nueva (8 hex), no la clave: "${guardada}"`);
    a.clave('1234');
    assert(PESTANAS.every(id => !a.visible(id)), 'Claves: la clave vieja ya no entra');
    a.clave('567890');
    assert(PESTANAS.every(id => a.visible(id)), 'Claves: la nueva si');
  }

  // 4. Olvido (D-57): la de soporte permite poner una nueva; una erronea no.
  {
    const a = montar();
    const CA = a.w.ClaveAdmin || { fnv1a: () => '', huella: () => '', HUELLA_SOPORTE: '' };  // ausente: rojo, no excepcion
    assert(CA.fnv1a('') === '811c9dc5' && CA.fnv1a('a') === 'e40c292c' && CA.fnv1a('foobar') === 'bf9cf968',
      'Claves: la huella es FNV-1a de 32 bits (vectores de referencia)');
    assert(/^[0-9a-f]{8}$/.test(CA.HUELLA_SOPORTE) && CA.HUELLA_SOPORTE !== CA.huella('1234'),
      `Claves: la de soporte se guarda solo como huella de 8 hex: ${CA.HUELLA_SOPORTE}`);
    a.el('btn-toggle-role').click();
    a.pulsa('btn-clave-olvido');
    assert(/soporte/i.test(a.txt('clave-texto')), 'Claves: «He olvidado la clave» pide la de soporte');
    a.clave('0000');
    assert(/incorrecta/i.test(a.txt('clave-msg')) && /soporte/i.test(a.txt('clave-texto')),
      'Claves: una clave de soporte erronea no deja poner otra');
    a.clave('1234');
    assert(/soporte/i.test(a.txt('clave-texto')), 'Claves: la de administracion no sirve de soporte');
    const sop = (typeof process !== 'undefined' && process.env.CLAVE_SOPORTE) || '';
    if (sop) {
      a.clave(sop);
      assert(/nueva/i.test(a.txt('clave-texto')), 'Claves: la de soporte (CLAVE_SOPORTE) deja poner una nueva');
      a.clave('24680');
      a.clave('24680');
      assert(PESTANAS.every(id => a.visible(id)), 'Claves: y con la nueva se entra');
    }
  }

  // 5. Practica sin poste (SPEC_4 9.10 y 9.14 (f); D-86, D-89, D-120): solo con la clave, nunca con enlace abierto, y
  // por el MISMO cable que un equipo: nada llega al puerto real y lo que entra lo compone el simulador (js/simulador.js).
  {
    const eventos = (a) => Array.from(a.d.querySelectorAll('#event-feed .event-item'))
                                .map(n => n.textContent.replace(/\s+/g, ' ')).join(' || ');
    const oculta = (a) => !!a.el('franja-simulado') && a.el('franja-simulado').hidden;
    const P = (a) => a.w.Practica || { entrar() {}, salir() {}, _tick() {}, FLOTA: [] };  // ausente: rojo
    const a = montar();
    const real = a.w.bluetoothSerial;
    const boton = a.el('btn-practica');
    P(a).entrar();
    assert(!!boton && !!boton.closest('#tab-diag') && a.w.bluetoothSerial === real && oculta(a),
      'Practica: «Practicar sin poste» esta en Diagnostico y sin la clave de administracion no entra');
    a.el('btn-toggle-role').click(); a.clave('1234');
    P(a).entrar();
    const prim = a.d.querySelector('#event-feed .event-item');
    assert(!!a.w.Practica && a.w.bluetoothSerial === real && oculta(a) && !!prim &&
           /practica/i.test(prim.textContent) && /red/.test(prim.outerHTML),
      `Practica: con un enlace abierto se rechaza, con una linea roja en Eventos: ${eventos(a).slice(0, 160)}`);
    a.el('btn-bt-disconnect').click();
    P(a).entrar();
    const filas = Array.from(a.d.querySelectorAll('#bt-modal .bt-device-item'));
    const rotulos = P(a).FLOTA.map(c => c.rotulo);
    assert(!oculta(a) && !!a.el('franja-simulado') && a.d.body.classList.contains('simulado') && a.activo('bt-modal') &&
           filas.length >= 4 && rotulos.length >= 4 && filas.every(f => rotulos.includes(f.getAttribute('data-name'))),
      `Practica: desconectado y con clave, franja SIMULADO y la flota en el modal de equipos (${filas.length} filas)`);
    const fila1 = filas.find(f => !!P(a).FLOTA[0] && f.getAttribute('data-mac') === P(a).FLOTA[0].direccion);  // la lista ordena
    if (fila1) fila1.click();
    const oidas = [], Pp = a.w.PuertoPractica;
    if (Pp && Pp._cb) { const cb = Pp._cb; Pp._cb = (t) => { oidas.push(String(t).trim()); cb(t); }; }
    P(a)._tick();
    assert(!!fila1 && /^MAESTRO/.test(a.txt('node-name')) && a.tramas.length === 0,
      `Practica: la fila del Maestro habla como un Maestro y el puerto real no recibe nada: "${a.txt('node-name')}" ${a.tramas.join(' | ')}`);
    const ack = '$ACK,CMD:VERSION,RESULT:OK,NODE:MAESTRO,FW:d810f81*24';   // el de la cinta del 06/10, con su *XX
    // TIMEOUT_ENLACE_MS de app.js: la VERSION se pide sola tras 5 s; se espera hasta 9 s a que llegue (con carga, el
    // temporizador de 5 s se retrasa y una espera fija de 5,4 s fallaba sin que nada estuviera mal).
    for (let i = 0; i < 90 && !oidas.includes(ack); i++) await new Promise(r => setTimeout(r, 100));
    assert(oidas.includes(ack) && /d810f81/.test(eventos(a)) && a.tramas.length === 0,
      `Practica: la VERSION que la app pide al enlazar la contesta el simulador con el $ACK de la cinta (FW d810f81): ` +
      `${oidas.filter(t => /VERSION/.test(t)).join(' | ') || 'ninguna'}`);
    a.pulsa('btn-franja-salir');
    assert(a.w.bluetoothSerial === real && oculta(a) && !a.d.body.classList.contains('simulado'),
      'Practica: Salir devuelve el puerto real y quita la franja y la clase');
    // Campo nuevo: cada CAMPO: del $STATUS que compone el firmware esta en el $STATUS que compone el simulador.
    const fs = require('fs');
    const path = require('path');
    ['MAESTRO', 'ESCLAVO'].forEach(nodo => {
      const cpp = path.join(__dirname, '..', '..', '..', '01_Firmware', nodo === 'MAESTRO' ? 'Maestro' : 'Esclavo',
                            'src', 'bluetooth.cpp');
      const lit = (fs.readFileSync(cpp, 'utf8').match(new RegExp('"\\$STATUS,NODE:' + nodo + ',[^"]*"')) || [''])[0];
      const campos = lit.replace(/"/g, '').split(',').slice(1).map(c => c.split(':')[0]);
      const sim = a.w.Simulador ? a.w.Simulador.crear({ postes: JSON.parse(JSON.stringify(P(a).FLOTA)) }) : null;
      const st = sim ? sim.postes.filter(c => c.nodo === nodo).map(c => sim.status(c)) : [];
      const faltan = campos.filter(c => !st.some(t => t.includes(',' + c + ':')));
      assert(campos.length >= 10 && st.length > 0 && faltan.length === 0,
        `Practica: los ${campos.length} campos del $STATUS del ${nodo} estan en el del simulador; faltan: ${faltan.join(',') || '-'}`);
    });
  }
};
