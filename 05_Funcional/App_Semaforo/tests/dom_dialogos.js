// ===== tests/dom_dialogos.js =====
// SPEC_4 9.1: ninguna pregunta sale por prompt/confirm/alert del navegador; sale por el dialogo propio en espanol.
// No corre sola: la llama test_dom_execution.js con SU montarAppLimpia y SU assert. Pulsa los botones REALES de
// app.js con los tres nativos espiados: si la app los llama, se cuenta y cae. Los nativos contestan "si" a todo,
// asi que con el codigo viejo los registros se borran sin que el dialogo propio llegue a abrirse.
// Lo que no mide: el telefono (idioma del sistema, teclado en pantalla).
module.exports = async function pruebaDialogos(montarAppLimpia, assert) {
  const a = montarAppLimpia();
  const w = a.w, d = a.d;
  const nativos = [];
  w.prompt = () => { nativos.push('prompt'); return 'Cruce Nativo'; };
  w.confirm = () => { nativos.push('confirm'); return true; };
  w.alert = () => { nativos.push('alert'); };
  const el = (id) => d.getElementById(id);
  const abierto = () => !!el('dialogo-modal') && el('dialogo-modal').classList.contains('active');
  const texto = () => (abierto() ? el('dialogo-modal').textContent.replace(/\s+/g, ' ') : '');
  const aceptar = (v) => {
    if (!abierto()) return;
    if (v !== undefined) el('dialogo-entrada').value = v;
    el('btn-dialogo-aceptar').click();
  };
  const cancelar = () => { if (abierto()) el('btn-dialogo-cancelar').click(); };

  // 1. Los tres registros que se vacian: el dialogo nombra lo que borra, Cancelar no borra, la accion si.
  if (!w.RegistroEnlace.cargar().registros.length) w.RegistroEnlace.anotar('MUESTRA', 97, 'prueba', Date.now());
  w.DiarioOrdenes.anotarOrden('VERSION', null, Date.now(), { salio: false, motivo: 'prueba' });
  const casos = [
    { boton: 'btn-registro-limpiar', que: /bit[aá]cora del enlace/i,
      cuenta: () => w.RegistroEnlace.cargar().registros.length },
    { boton: 'btn-depu-limpiar', que: /cinta de tramas/i, cuenta: () => w.RegistroCrudo.todas().length },
    { boton: 'btn-diario-limpiar', que: /diario de [oó]rdenes/i, cuenta: () => w.DiarioOrdenes.todas().length }
  ];
  casos.forEach(c => {
    const antes = c.cuenta();
    el(c.boton).click();
    assert(abierto() && c.que.test(texto()),
      `Dialogos: ${c.boton} abre el dialogo propio y dice que borra: "${texto().slice(0, 90)}"`);
    cancelar();
    assert(!abierto() && antes > 0 && c.cuenta() === antes,
      `Dialogos: ${c.boton} con Cancelar no borra nada (${antes} -> ${c.cuenta()})`);
    el(c.boton).click();
    const etiq = abierto() ? el('btn-dialogo-aceptar').textContent.trim() : '';
    assert(/vaciar|borrar/i.test(etiq) && !/^(ok|aceptar)$/i.test(etiq),
      `Dialogos: ${c.boton} el boton nombra la accion, no "OK"/"Aceptar": "${etiq}"`);
    aceptar();
    assert(!abierto() && c.cuenta() === 0, `Dialogos: ${c.boton} con la accion borra (${c.cuenta()})`);
  });

  // 2. Tocar fuera es cancelar; Cancelar a la izquierda de la accion, como en el aviso de via.
  w.DiarioOrdenes.anotarOrden('VERSION', null, Date.now(), { salio: false, motivo: 'prueba' });
  el('btn-diario-limpiar').click();
  const ca = el('btn-dialogo-cancelar'), ac = el('btn-dialogo-aceptar');
  assert(!!ca && !!ac && /cancelar/i.test(ca.textContent) &&
         !!(ca.compareDocumentPosition(ac) & w.Node.DOCUMENT_POSITION_FOLLOWING),
    'Dialogos: Cancelar va antes (a la izquierda) que la accion');
  if (abierto()) el('dialogo-modal').dispatchEvent(new w.MouseEvent('click', { bubbles: true }));
  assert(!abierto() && w.DiarioOrdenes.todas().length === 1, 'Dialogos: tocar fuera cierra sin borrar');

  // 3. Crear un cruce: dos preguntas; cancelar la primera o un nombre vacio no crea; cancelar la segunda guarda.
  const nCruces = () => w.SiteManager.obtenerCruces().length;
  const n0 = nCruces();
  el('btn-open-add-site').click();
  assert(abierto() && /nombre/i.test(texto()) && !!el('dialogo-entrada') && !el('dialogo-entrada').hidden &&
         el('dialogo-entrada').value === '',
    `Dialogos: crear cruce pide el nombre en un campo VACIO, sin nombre inventado: "${texto().slice(0, 60)}"`);
  cancelar();
  el('btn-open-add-site').click(); aceptar('   '); cancelar();
  assert(nCruces() === n0, `Dialogos: cancelar o un nombre vacio no crea cruce (${n0} -> ${nCruces()})`);
  el('btn-open-add-site').click(); aceptar('Cruce Dialogo Km 7');
  assert(abierto() && /ubicaci[oó]n/i.test(texto()) && el('dialogo-entrada').value === '',
    `Dialogos: tras el nombre pide la ubicacion, en un campo vacio: "${texto().slice(0, 60)}"`);
  aceptar('PR 7+000');
  const creado = w.SiteManager.obtenerCruces().find(s => /Cruce Dialogo Km 7/.test(s.nombre)) || {};
  assert(nCruces() === n0 + 1 && creado.ubicacion === 'PR 7+000' &&
         /Cruce Dialogo Km 7/.test(el('current-site-name').textContent),
    `Dialogos: nombre y ubicacion guardan el cruce y lo activan (${creado.nombre} / ${creado.ubicacion})`);
  el('btn-open-add-site').click(); aceptar('Cruce Sin PR'); cancelar();
  const sinPr = w.SiteManager.obtenerCruces().find(s => /Cruce Sin PR/.test(s.nombre));
  assert(!!sinPr && /no especificada/i.test(sinPr.ubicacion),  // el rotulo de SiteManager.agregarCruce para ''
    `Dialogos: cancelar la ubicacion guarda el cruce sin ella (${sinPr ? sinPr.ubicacion : 'no creado'})`);

  // 4. Renombrar: el campo trae el nombre actual; vacio no renombra; Guardar renombra.
  el('btn-select-site').click();
  const tarjeta = Array.from(d.querySelectorAll('.site-card')).find(c => /Cruce Dialogo Km 7/.test(c.textContent));
  if (tarjeta) tarjeta.querySelector('.btn-edit-site').click();
  assert(abierto() && /Cruce Dialogo Km 7/.test(el('dialogo-entrada').value),
    `Dialogos: renombrar trae el nombre actual en el campo: "${abierto() ? el('dialogo-entrada').value : ''}"`);
  aceptar('');
  const tarj2 = Array.from(d.querySelectorAll('.site-card')).find(c => /Cruce Dialogo Km 7/.test(c.textContent));
  if (tarj2) tarj2.querySelector('.btn-edit-site').click();
  aceptar('Cruce Dialogo Renombrado');
  assert(w.SiteManager.obtenerCruces().some(s => /Cruce Dialogo Renombrado/.test(s.nombre)) &&
         !w.SiteManager.obtenerCruces().some(s => /Cruce Dialogo Km 7/.test(s.nombre)),
    'Dialogos: vacio no renombra; Guardar si');

  assert(nativos.length === 0, `Dialogos: ningun prompt/confirm/alert del navegador (${nativos.join(', ') || 'ninguno'})`);
};
