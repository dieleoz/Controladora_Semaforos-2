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
};
