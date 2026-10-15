// ===== tests/dom_exportar.js =====
// La prueba jsdom del boton Exportar todo. No corre sola: la llama test_dom_execution.js
// con SU montarAppLimpia y SU assert, asi que sus aserciones cuentan en el mismo
// RESULTADO JSDOM que lee la compuerta.
// EXPORTAR TODO DENTRO DE LA APK (28/09): el boton real de app.js contra dobles de
// @capacitor/filesystem y @capacitor/share colgados de window.Capacitor.Plugins, que es
// donde los publica el puente nativo (JSExport.getPluginJS de @capacitor/android 6.2.1).
// Se exigen las dos respuestas: con los plugins sale el ADJUNTO y no hay rojo; sin ellos,
// o si la hoja falla, rojo en la bitacora. Lo que no mide: un telefono real (SPEC_4 7.12).
module.exports = async function pruebaExportarApk(montarAppLimpia, assert) {
  const esperar = () => new Promise(r => setTimeout(r, 0));
  const eventos = (a) => Array.from(a.d.querySelectorAll('.event-item'))
                              .map(n => n.textContent.replace(/\s+/g, ' ')).join(' || ');
  const montar = (plugins) => {
    const a = montarAppLimpia();
    a.w.Capacitor = { isNativePlatform: () => true, Plugins: plugins };
    return a;
  };

  const llam = { write: [], share: [] };
  const ok = montar({
    Filesystem: { writeFile: (x) => { llam.write.push(x); return Promise.resolve({ uri: 'file:///c/' + x.path }); } },
    Share: { share: (x) => { llam.share.push(x); return Promise.resolve({ activityType: 'com.whatsapp' }); } }
  });
  ok.d.getElementById('btn-exportar-todo').click();
  await esperar();
  const w0 = llam.write[0] || {};
  assert(llam.write.length === 1 && w0.directory === 'CACHE' && /^IOTVIAL_.*\.txt$/.test(w0.path) &&
         /Firmware:/.test(w0.data || ''),
    `Exportar APK: escribe IOTVIAL_*.txt en CACHE con la linea Firmware: (${w0.path})`);
  assert(llam.share.length === 1 && (llam.share[0].files || [])[0] === 'file:///c/' + w0.path,
    'Exportar APK: la hoja de compartir recibe el fichero como adjunto');
  assert(!/EXPORTAR COMO ARCHIVO/.test(eventos(ok)) &&
         /pasado a la app elegida/.test(ok.d.getElementById('toast-msg').textContent),
    'Exportar APK: sin rojo, y el toast no afirma que el mensaje llego');

  const falla = montar({
    Filesystem: { writeFile: () => Promise.resolve({ uri: 'file:///c/a.txt' }) },
    Share: { share: () => Promise.reject(new Error('only file urls are supported')) }
  });
  falla.d.getElementById('btn-exportar-todo').click();
  await esperar();
  assert(/EXPORTAR COMO ARCHIVO fallo: only file urls are supported/.test(eventos(falla)),
    'Exportar APK: si la hoja falla, rojo con el motivo');

  const vieja = montar({});
  vieja.d.getElementById('btn-exportar-todo').click();
  await esperar();
  assert(/EXPORTAR COMO ARCHIVO fallo: esta APK no trae la pieza nativa/.test(eventos(vieja)),
    'Exportar APK sin plugins: el rojo de siempre, no un boton mudo');

  // SPEC_4 9.10 (d), D-89: en la practica sin poste el fichero se llama SIMULADO, abre con la marca y la sesion de
  // EXTERNAL (D-55) no escribe nada.
  const lp = { write: [], append: [] };
  const pr = montar({
    Filesystem: { writeFile: (x) => { lp.write.push(x); return Promise.resolve({ uri: 'file:///c/' + x.path }); },
                  appendFile: (x) => { lp.append.push(x); return Promise.resolve(); } },
    Share: { share: () => Promise.resolve({}) }
  });
  const g = (id) => pr.d.getElementById(id);
  g('btn-toggle-role').click(); g('clave-entrada').value = '1234'; g('btn-clave-entrar').click();
  g('btn-bt-disconnect').click();
  const P = pr.w.Practica || { entrar() {}, salir() {}, _tick() {} };
  P.entrar();
  await esperar();
  lp.write.length = 0; lp.append.length = 0;
  const fila = pr.d.querySelector('#bt-modal .bt-device-item[data-mac="SIM:1"]');
  if (fila) fila.click();
  for (let i = 0; i < 60; i++) P._tick();
  g('btn-exportar-todo').click();
  await esperar();
  const wp = lp.write.find(x => x.directory === 'CACHE') || {};
  assert(!!pr.w.Practica && lp.append.length === 0 && !lp.write.some(x => x.directory === 'EXTERNAL') &&
         /^IOTVIAL_SIMULADO_.*\.txt$/.test(wp.path || '') && /^Simulado: SI/.test(wp.data || ''),
    `Exportar en practica: IOTVIAL_SIMULADO_*.txt que abre con "Simulado: SI" y nada en EXTERNAL (${wp.path}, ` +
    `${lp.append.length} append)`);
  P.salir();
};
