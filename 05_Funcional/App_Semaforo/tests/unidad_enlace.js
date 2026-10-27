// Pruebas N-181 B, sacadas de test_unitarios_app.js (no crece: tope de 500). Las registra su runTest.
const assert = require('assert');
const fs = require('fs');
const path = require('path');
module.exports = function (runTest, calcularChecksumNmea) {
const __dirname = path.join(__filename, '..', '..');
// =============================================================================
// N-181 B: app.js REAL en jsdom, enlace que se suelta y se vuelve a abrir
// =============================================================================
// SPEC_4 3.ter.bis (D-130): la espera del $ACK diferido de SET_DEG_AUTO es de la conexion que envio la orden.
// SPEC_4 9.14 (c) (D-129): los avisos de grupo salen una vez por CONEXION, tambien al reenlazar la misma MAC.
// Montaje minimo, el de montarAppLimpia() de test_dom_execution.js: TODO SINCRONO (tras un await jsdom arrancaria
// otra app debajo). Sin jsdom (worktree sin node_modules) estas pruebas FALLAN: no se dan por buenas en silencio.
const MAC_B = '00:11:22:33:44:55';
function montarAppB(precarga) {
  const { JSDOM } = require('jsdom');
  const html = fs.readFileSync(path.join(__dirname, 'index.html'), 'utf8');
  const w = new JSDOM(html, { runScripts: 'dangerously', url: 'http://localhost/' }).window;
  w.addEventListener('DOMContentLoaded', e => e.isTrusted && e.stopImmediatePropagation(), true);
  Object.keys(precarga || {}).forEach(k => w.localStorage.setItem(k, precarga[k]));
  w.navigator.vibrate = () => true;
  const tramas = [], borrados = [];
  w.bluetoothSerial = { isEnabled: (ok) => ok(), connect: (mac, ok) => ok(), disconnect: (ok) => ok && ok(),
    list: (ok) => ok([{ name: 'JDY-31 Maestro', id: MAC_B, address: MAC_B }]),
    subscribe: (delim, cb) => { w._btSubscribeCb = cb; }, write: (t, ok) => { tramas.push(t); if (ok) ok(); } };
  (html.match(/src="(js\/[^"]+\.js)"/g) || []).map(m => m.replace(/src="|"/g, '')).forEach(rel => {
    const src = fs.readFileSync(path.join(__dirname, rel), 'utf8');
    const nombres = (src.match(/^(?:const|class|function|var|let)\s+([A-Za-z_$][\w$]*)/gm) || []).map(m => m.split(/\s+/)[1]);
    w.eval(src + ';' + nombres.map(n => `try{window.${n}=${n};}catch(e){}`).join(''));
  });
  w.eval(fs.readFileSync(path.join(__dirname, 'app.js'), 'utf8'));
  w.document.dispatchEvent(new w.Event('DOMContentLoaded'));
  const el = (id) => w.document.getElementById(id);
  const a = { w, el, tramas, borrados,
    entra: (p) => w._btSubscribeCb(`$${p}*${calcularChecksumNmea(p)}\n`),
    enlazar: () => { el('btnDevice').click(); w.document.querySelector('.bt-device-item').click(); },
    status: () => a.entra('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:14:31:00'),
    soltar: () => el('btn-bt-disconnect').click(),
    eventos: () => Array.from(w.document.querySelectorAll('#event-feed .event-item')).map(e => e.textContent) };
  a.enlazar(); a.status(); w.DegAuto._estado = { este: false };
  const borrar = w.clearTimeout; w.clearTimeout = (id) => { borrados.push(id); return borrar(id); };
  return a;
}

runTest('N-181 B', 'D-129: soltar y reenlazar la misma MAC repite el aviso rojo de NODE: (9.14 (c))', () => {
  const grupos = { v: 1, grupos: [{ id: 'g-b', via: 'Via 1', nombre: 'Km 20', pr: '', MAESTRO: null,
                   ESCLAVO: { mac: MAC_B, serie: 'SEM-M-01', fw: null, fecha: 'F' } }] };
  const a = montarAppB({ iot_vial_grupos_v1: JSON.stringify(grupos) });
  const avisos = () => a.eventos().filter(t => /NODE:MAESTRO y está matriculado como ESCLAVO/.test(t)).length;
  assert.strictEqual(avisos(), 1, 'el primer enlace da el aviso rojo de NODE: una vez');
  a.status();
  assert.strictEqual(avisos(), 1, 'dentro del mismo enlace un aviso ya dado no se repite');
  a.soltar(); a.enlazar(); a.status();
  assert.strictEqual(avisos(), 2, 'reenlazada la misma MAC, el aviso sale otra vez: la cuenta es de la conexion');
});

runTest('N-181 B', 'D-130: soltar el enlace cancela la espera de SET_DEG_AUTO y lo deja en el registro', () => {
  const a = montarAppB();
  const real = a.w.setTimeout, armados = [];
  a.w.setTimeout = (fn, ms) => { const id = real(fn, ms); armados.push({ id, ms }); return id; };
  a.el('btn-degauto-on').click();
  ['1', '2', '3', '4'].forEach(k => a.w.document.querySelector(`.pin-btn[data-key="${k}"]`).click());
  a.w.setTimeout = real;
  assert.ok(a.tramas.includes('CMD:PIN:1234:SET_DEG_AUTO:1\r\n'), 'sale la orden: ' + a.tramas.join(' | '));
  const espera = armados.find(t => t.ms === a.w.DegAuto.ESPERA_MS);
  assert.ok(espera && a.w.DegAuto._espera, 'queda esperando el $ACK diferido');
  a.soltar();
  assert.strictEqual(a.w.DegAuto._espera, null, 'al soltar el enlace la espera se cancela');
  assert.ok(a.borrados.includes(espera.id), 'su temporizador se borra: no pinta "no contesto" en la sesion nueva');
  assert.ok(a.eventos().some(t => t.includes('SET_DEG_AUTO:1 enviada a ' + MAC_B +
    ': sin acuse en esa conexion, mire la consulta')), 'linea del registro (SPEC_4 3.ter.bis): ' + a.eventos()[0]);
  a.enlazar(); a.status();
  assert.ok(!a.el('btn-degauto-on').disabled, 'en la conexion nueva el boton no sigue esperando');
});

runTest('N-181 B', 'D-130: un $ACK de SET_DEG_AUTO sin espera en esta conexion se pinta y dispara CONSULTA_DEG_AUTO', () => {
  const a = montarAppB();
  a.tramas.length = 0;
  a.entra('ACK,CMD:SET_DEG_AUTO,RESULT:ON_EFECTIVO');
  assert.ok(/Equipo: degradado automatico ACTIVO/.test(a.eventos()[0] || ''), 'se pinta lo que dice el equipo: ' + a.eventos()[0]);
  assert.deepStrictEqual(a.tramas, ['CMD:CONSULTA_DEG_AUTO\r\n'], 'y se consulta, sin PIN');
});
};
