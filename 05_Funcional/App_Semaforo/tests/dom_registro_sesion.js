// ===== tests/dom_registro_sesion.js =====
// SPEC_4 9.3 (D-55, parte de la app): las cuatro mejoras del registro del 06/10, sobre app.js y los js/ REALES.
// No corre sola: la llama test_dom_execution.js con SU montarAppLimpia y SU assert. Bordes de fuera: la cinta del
// 06/10 (evidencia/061020261110) tiro 1542 tramas por el tope de 300; el appId de capacitor.config.json; la APK
// escribe por @capacitor/filesystem (CACHE, EXTERNAL) y no por <a download> (SPEC_4 7.12).
// Lo que no mide: el telefono (que copy/appendFile sobre EXTERNAL funcionen y que adb lo lea).
const fs = require('fs');
const path = require('path');

function xor(p) {
  let c = 0;
  for (const ch of p) c ^= ch.charCodeAt(0);
  return c.toString(16).toUpperCase().padStart(2, '0');
}
const status = (node, modo, estado) => {
  const p = `STATUS,NODE:${node},SERIE:SEM-M-01,MODO:${modo},ESTADO:${estado},T:31,RF:97,RTT:70,BAT:12.9,HORA:14:31:00`;
  return `$${p}*${xor(p)}\n`;
};
const esperar = () => new Promise(r => setTimeout(r, 0));
const eventos = (x) => Array.from(x.d.querySelectorAll('.event-item'))
  .map(n => n.textContent.replace(/\s+/g, ' ')).join(' || ');

// Dobles de @capacitor/filesystem y @capacitor/share que apuntan el orden de las llamadas.
function dobles(o) {
  const ll = { write: [], copy: [], share: [], orden: [] };
  const p = {
    Filesystem: {
      writeFile: (x) => {
        ll.write.push(x); ll.orden.push('write');
        return Promise.resolve({ uri: 'file:///c/' + x.path });
      },
      copy: (x) => {
        ll.copy.push(x); ll.orden.push('copy');
        return o.fallaCopy ? Promise.reject(new Error(o.fallaCopy)) : Promise.resolve({ uri: 'file:///e/' + x.to });
      }
    },
    Share: { share: (x) => {
      ll.share.push(x); ll.orden.push('share');
      return o.cancela ? Promise.reject(new Error('Share canceled')) : Promise.resolve({});
    } }
  };
  return { ll, p };
}

async function exportar(x, o) {
  const db = dobles(o || {});
  x.w.Capacitor = { isNativePlatform: () => true, Plugins: db.p };
  x.d.getElementById('btn-exportar-todo').click();
  await esperar(); await esperar();
  return db.ll;
}

module.exports = async function pruebaRegistroSesion(montar, assert) {
  // ---- 9.3.1 Los cambios de $STATUS no se van con el tope de la cinta ----
  const a = montar();
  const w = a.w;
  const RC = w.RegistroCambios;
  assert(!!RC, 'Registro 9.3.1: existe RegistroCambios, aparte de la cinta');
  if (RC) {
    assert(JSON.stringify(RC.CAMPOS) === JSON.stringify(w.DiarioOrdenes.CAMPOS_EFECTO),
      `Registro 9.3.1: mira los mismos campos que el efecto del diario (${RC.CAMPOS})`);
    const tope = w.RegistroCrudo.TOPE;
    for (let i = 0; i < tope + 21; i++) w._btSubscribeCb(status('MAESTRO', 'AMBAR', 'AMBAR_INT'));
    const enCinta = w.RegistroCrudo.todas().some(r => /MODO:AUTO/.test(r.linea));
    const cambio = RC.todas().find(r => r.clase === 'CAMBIO' &&
      r.cambios.some(c => c.campo === 'MODO' && c.de === 'AUTO' && c.a === 'AMBAR'));
    assert(w.RegistroCrudo.descartados > 0 && !enCinta && !!cambio && /MODO:AMBAR/.test(cambio.linea),
      `Registro 9.3.1: la cinta roto (${w.RegistroCrudo.descartados} tiradas) y AUTO -> AMBAR sigue, con su trama`);
    const n = RC.todas().filter(r => r.clase === 'CAMBIO').length;
    assert(n === 1, `Registro 9.3.1: ${tope + 20} $STATUS iguales no son cambios (${n} cambio)`);
    w._btSubscribeCb(status('ESCLAVO', 'AUTO', 'R1_V2'));
    const ini = RC.todas().filter(r => r.clase === 'INICIO').map(r => r.node).join(',');
    assert(ini === 'MAESTRO,ESCLAVO' && RC.todas().filter(r => r.clase === 'CAMBIO').length === 1,
      `Registro 9.3.1: el primer $STATUS de cada NODE es INICIO, no un cambio del otro poste (${ini})`);
  }

  // ---- 9.3.2 y 9.3.3 Exportar en la APK: copia fuera de CACHE, y todo en un solo .txt ----
  w.RegistroEnlace.anotar('CAIDA', { medido: false }, 'se fue el enlace (prueba 9.3)', Date.now());
  const ll = await exportar(a, { cancela: true });
  const c0 = ll.copy[0] || {};
  assert(ll.copy.length === 1 && c0.directory === 'CACHE' && c0.toDirectory === 'EXTERNAL' &&
         c0.from === (ll.write[0] || {}).path && c0.to === c0.from && ll.orden.join(',') === 'write,copy,share',
    `Registro 9.3.2: copia el .txt de CACHE a EXTERNAL antes de la hoja (${ll.orden.join(',')})`);
  assert(/copia/i.test(eventos(a)) && /Android\/data\/com\.iotvial\.semaforos\/files\//.test(eventos(a)),
    'Registro 9.3.2: la bitacora dice donde quedo la copia, aunque se cancele la hoja');
  const appId = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'capacitor.config.json'), 'utf8')).appId;
  assert(w.Exportar.RUTA_EXTERNA === 'Android/data/' + appId + '/files/',
    `Registro 9.3.2: la ruta que se le dice al tecnico es la del appId de capacitor.config.json (${appId})`);
  const txt = (ll.write[0] || {}).data || '';
  const i = ['DIARIO DE ORDENES', 'CINTA DE TRAMAS EN CRUDO', 'CAMBIOS DE ESTADO', 'REGISTRO DEL ENLACE']
    .map(s => txt.indexOf('IOT-VIAL - ' + s));
  assert(i[0] >= 0 && i[1] > i[0] && i[2] > i[1] && i[3] > i[2],
    `Registro 9.3.3: un solo .txt con diario, cinta, cambios y enlace, en ese orden (${i})`);
  assert(txt.indexOf('Fecha_ISO,Hora_local,Clase,Enlace_pct,RTT_ms,Detalle', i[3]) > 0 &&
         /"CAIDA",,,"se fue el enlace \(prueba 9\.3\)"/.test(txt.slice(i[3])) &&
         /MODO: AUTO -> AMBAR/.test(txt.slice(i[2], i[3])),
    'Registro 9.3.3: el enlace sale con las filas del CSV de siempre y el cambio con su campo');
  const b = montar();
  const llF = await exportar(b, { fallaCopy: 'disco lleno' });
  assert(llF.share.length === 1 && /COPIA.*no se guardo.*disco lleno/i.test(eventos(b)),
    'Registro 9.3.2: si la copia falla, rojo con el motivo y el envio sigue');
  b.w.RegistroCrudo.limpiar(); b.w.DiarioOrdenes.limpiar();
  if (b.w.RegistroCambios) b.w.RegistroCambios.limpiar();
  b.w.RegistroEnlace.anotar('CAIDA', { medido: false }, 'solo enlace', Date.now());
  const llE = await exportar(b, {});
  assert(llE.write.length === 1, 'Registro 9.3.3: con solo registro del enlace exporta, no "nada que exportar" (' +
    b.d.getElementById('toast-msg').textContent + ')');

  // ---- 9.3.4 En la APK, a fichero desde el principio de la sesion ----
  const c = montar();
  const RS = c.w.RegistroSesion;
  assert(!!RS, 'Registro 9.3.4: existe RegistroSesion');
  if (!RS) return;
  c.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  await RS.volcar();
  assert(RS.estado === 'APAGADO', `Registro 9.3.4: fuera de la APK no escribe nada (${RS.estado})`);
  const fl = { write: [], append: [] };
  const FS = {
    writeFile: (x) => { fl.write.push(x); return Promise.resolve({ uri: 'file:///e/' + x.path }); },
    appendFile: (x) => { fl.append.push(x); return Promise.resolve(); }
  };
  c.w.Capacitor = { isNativePlatform: () => true, Plugins: { Filesystem: FS } };
  c.w._btSubscribeCb(status('MAESTRO', 'AMBAR', 'AMBAR_INT'));
  c.w.RegistroCrudo.anotar('CMD:PIN:1234:SET_MODO:AUTO\r\n', { enviada: true, tipo: 'CMD' });
  c.w.RegistroEnlace.anotar('CAIDA', { medido: false }, 'caida en sesion', Date.now());
  await RS.volcar();
  const w0 = fl.write[0] || {};
  const todo = (w0.data || '') + fl.append.map(x => x.data).join('');
  assert(fl.write.length === 1 && w0.directory === 'EXTERNAL' && /^SESION_\d{8}-\d{6}\.txt$/.test(w0.path) &&
         fl.append.length > 0 && fl.append.every(x => x.path === w0.path && x.directory === 'EXTERNAL'),
    `Registro 9.3.4: abre SESION_<fecha>.txt en EXTERNAL y le anade (${w0.path}, ${fl.append.length} lotes)`);
  assert(/MODO:AMBAR/.test(todo) && /caida en sesion/.test(todo) &&
         /CMD:PIN:\*\*\*\*:SET_MODO:AUTO/.test(todo) && !/1234/.test(todo),
    'Registro 9.3.4: van la trama, la caida del enlace y la orden con el PIN tapado');
  FS.appendFile = () => Promise.reject(new Error('sin sitio'));
  c.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  await RS.volcar();
  assert(RS.estado === 'FALLO' && /FALLO.*sin sitio/.test(c.w.Exportar.textoAnexos()),
    `Registro 9.3.4: un fallo de escritura para el registro y el .txt exportado lo declara (${RS.estado})`);
  const d = montar();
  const RS2 = d.w.RegistroSesion;
  RS2.TOPE_BYTES = 600;
  FS.appendFile = (x) => { fl.append.push(x); return Promise.resolve(); };
  d.w.Capacitor = c.w.Capacitor;
  for (let k = 0; k < 20; k++) d.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  await RS2.volcar();
  assert(RS2.estado === 'LLENO' && RS2.bytes <= RS2.TOPE_BYTES && /LLENO/.test(d.w.Exportar.textoAnexos()),
    `Registro 9.3.4: al tope de bytes para y lo dice (${RS2.estado}, ${RS2.bytes} de ${RS2.TOPE_BYTES} B)`);

  // ---- 9.8 El registro del puente (D-75): lo pide solo si el parte trae LOG:, recuerda por SERIE ----
  // Bordes de fuera: los literales de SPEC_4 9.8 (CMD:LOG,DESDE:<N>, el $ACK con sus diez campos, EVT:LOG con
  // D: codificado , -> ; y : -> =) y la clave iotvial_log_equipo_v1:<SERIE> de D-75.
  const trama = (p) => `$${p}*${xor(p)}\n`;
  const pide = (n) => JSON.stringify(['CMD:LOG,DESDE:' + n + '\r\n']);
  const parte = (log) => trama('EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:SOFTWARE,ARRANQUES:3,PERRO:ARMADO,WDT_MS:2000' +
    (log === undefined ? '' : ',LOG:' + log));
  const ack = (de, ha, ult, pr = 1) => trama(`ACK,NODE:PUENTE,CMD:LOG,RESULT:OK,DESDE:${de},HASTA:${ha},ULT:${ult},` +
    `PRIMERO:${pr},CONG:3,CRC:0,LARGO:0,ANILLO:0,FLASH:0,MALOS:0`);
  const reg = (n, d) => trama(`EVENT,NODE:PUENTE,EVT:LOG,N:${n},UP:${10 + n},FECHA:2026-10-14,HORA:10:00:0${n},C:E,` +
    'D:' + d);
  const logs = (t) => t.filter(x => /CMD:LOG/.test(x));
  const e = montar();
  e.w._btSubscribeCb(parte()); e.tramas.length = 0;
  e.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  assert(!!e.w.RegistroEquipo && logs(e.tramas).length === 0,
    `Registro 9.8: existe RegistroEquipo y un parte sin LOG: (puente viejo) no pide nada (${logs(e.tramas)})`);
  const f = montar();
  f.w._btSubscribeCb(parte(7)); f.tramas.length = 0;
  f.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  assert(JSON.stringify(logs(f.tramas)) === pide(1),
    `Registro 9.8: parte con LOG:7 y $STATUS con SERIE escriben CMD:LOG,DESDE:1 (${JSON.stringify(f.tramas)})`);
  const antes = f.d.querySelectorAll('.event-item').length;
  f.w._btSubscribeCb(ack(1, 7, 7));
  for (let n = 1; n <= 7; n++) {
    f.w._btSubscribeCb(reg(n, n === 4 ? 'ALARM;NODE=MAESTRO;TIPO=LIMITE;DETALLE=X' : 'ACK;CMD=SET_MODO;RESULT=OK'));
  }
  const ev = Array.from(f.d.querySelectorAll('.event-item')).map(x => x.textContent.replace(/\s+/g, ' '));
  const deLog = ev.filter(t => /Equipo \[LOG\] N:\d/.test(t));
  assert(deLog.length === 7 && deLog.some(t => /ALARM,NODE:MAESTRO,TIPO:LIMITE/.test(t)) &&
         ev.length - antes <= 8 && !ev.some(t => /MAESTRO/.test(t) && !/\[LOG\]/.test(t)) &&
         !f.w.AvisoCorte._cartel,
    `Registro 9.8: siete lineas [LOG] decodificadas y ningun aviso de alarma ni de corte (${deLog.length}, ` +
    `${ev.length - antes} nuevas)`);
  const clave = 'iotvial_log_equipo_v1:SEM-M-01';
  assert(f.w.localStorage.getItem(clave) === '7',
    `Registro 9.8: recuerda 7 para SEM-M-01 (${f.w.localStorage.getItem(clave)})`);
  f.w._btSubscribeCb(parte(9)); f.tramas.length = 0;
  f.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  assert(JSON.stringify(logs(f.tramas)) === pide(8),
    `Registro 9.8: con 7 recordado y LOG:9 pide DESDE:8 (${JSON.stringify(logs(f.tramas))})`);
  f.tramas.length = 0;
  f.w._btSubscribeCb(ack(8, 0, 3));
  f.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));   // el $ACK no envia (app_05): sale con el $STATUS
  f.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));   // y una sola vez
  assert(f.w.localStorage.getItem(clave) === '0' &&
         JSON.stringify(logs(f.tramas)) === pide(1),
    `Registro 9.8: ULT:3 con 7 recordado vuelve a 0 y pide DESDE:1 (${f.w.localStorage.getItem(clave)}, ` +
    `${JSON.stringify(logs(f.tramas))})`);
  const llR = await exportar(f, {});
  const tx = (llR.write[0] || {}).data || '';
  const iC = tx.indexOf('IOT-VIAL - CAMBIOS DE ESTADO');
  const iR = tx.indexOf('IOT-VIAL - REGISTRO DEL EQUIPO');
  assert(iC >= 0 && iR > iC && /CONG:3/.test(tx.slice(iR)) && /ALARM,NODE:MAESTRO/.test(tx.slice(iR)),
    `Registro 9.8: el .txt trae REGISTRO DEL EQUIPO detras de CAMBIOS DE ESTADO, con CONG:3 (${iC}, ${iR})`);
  // (7) D-100: lo recordado es el ultimo N RECIBIDO; PRIMERO > recordado + 1 deja recordado = PRIMERO - 1.
  const g = montar();
  g.w._btSubscribeCb(parte(40)); g.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  const evG = [];                                   // Eventos guarda 30: se mira tras cada trama
  const verG = (t) => { g.w._btSubscribeCb(t);
    evG.push(Array.from(g.d.querySelectorAll('.event-item')).map(x => x.textContent)); };
  const cuenta = (l, rx) => l.filter(t => rx.test(t)).length;
  verG(ack(5, 40, 40, 5));
  for (let n = 5; n <= 40; n++) verG(reg(n, 'ACK;CMD=SET_MODO;RESULT=OK'));
  g.w._btSubscribeCb(parte(40)); g.tramas.length = 0;
  g.w._btSubscribeCb(status('MAESTRO', 'AUTO', 'V1_R2'));
  assert(g.w.localStorage.getItem(clave) === '40' && cuenta(evG[0], /ya no tiene/) === 1 &&
         evG.every(l => cuenta(l, /ya no tiene/) <= 1 && cuenta(l, /hueco/) === 0) && logs(g.tramas).length === 0,
    `Registro 9.8 (7) D-100: PRIMERO:5 y 36 entradas dejan 40 recordado, un aviso, y LOG:40 no pide ` +
    `(${g.w.localStorage.getItem(clave)}, ${JSON.stringify(logs(g.tramas))})`);
};
