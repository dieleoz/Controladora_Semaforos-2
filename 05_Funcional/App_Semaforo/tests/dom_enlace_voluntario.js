// ===== tests/dom_enlace_voluntario.js =====
// «ENLACE PERDIDO» SOLO SOBRE UN ENLACE ABIERTO (SPEC_4 9.9 (d), 16/10). Medido en el telefono del responsable en la
// practica: a los 3,5 s de desconectar A PROPOSITO salia en rojo «Enlace perdido» y se anotaba una CAIDA, y conectando
// a otro poste dentro de esa ventana el vigilante cortaba el enlace nuevo a 1 s de abrirlo (10:07:06 -> 10:07:07).
// Causa: olvidarEnlace() no apagaba telemetriaViva ni el reloj del ultimo $STATUS. El reloj se adelanta con Date.now
// y el vigilante real (setInterval de 1 s) hace el resto: se mide el CAMINO, no la variable de dentro de la IIFE.
module.exports = async function pruebaEnlaceVoluntario(montarAppLimpia, assert) {
  const espera = (ms) => new Promise(r => setTimeout(r, ms));
  const eventos = (a) => Array.from(a.d.querySelectorAll('.event-item')).map(n => n.textContent.replace(/\s+/g, ' '));
  const perdido = (a) => eventos(a).filter(t => /Enlace perdido/.test(t)).length;

  // (a) La otra respuesta: el enlace ABIERTO que calla mas de 5 s SI declara la caida.
  const a = montarAppLimpia();
  const relojA = a.w.Date.now, t0 = relojA();
  a.w.Date.now = () => t0 + 6000;
  await espera(1300);
  assert(perdido(a) === 1, `Enlace voluntario (a): un enlace abierto que calla 6 s SI da «Enlace perdido»: ${perdido(a)}`);
  a.w.Date.now = relojA;

  // (b) Desconectar a proposito: ni alarma ni CAIDA, aunque pasen 6 s sin $STATUS.
  const b = montarAppLimpia();
  const relojB = b.w.Date.now, t1 = relojB();
  b.d.getElementById('btn-bt-disconnect').click();
  b.w.Date.now = () => t1 + 6000;
  await espera(1300);
  assert(perdido(b) === 0,
    `Enlace voluntario (b): tras DESCONECTAR a proposito no sale «Enlace perdido»: ${eventos(b).slice(0, 2).join(' || ')}`);

  b.w.Date.now = relojB;

  // (c) Desconectar y abrir otro enlace ENSEGUIDA, antes de que el vigilante pase: el enlace nuevo no ha mandado aun su
  // primer $STATUS y no lo corta la caida del anterior (el caso de las 10:07 en el telefono).
  const c = montarAppLimpia();
  const relojC = c.w.Date.now, t2 = relojC();
  c.d.getElementById('btn-bt-disconnect').click();
  c.d.getElementById('btnDevice').click();
  c.d.querySelector('.bt-device-item').click();
  c.w.Date.now = () => t2 + 6000;
  await espera(1300);
  assert(c.w.appEnlazada() === true && perdido(c) === 0,
    `Enlace voluntario (c): el enlace nuevo sigue abierto sin su primer $STATUS: enlazada=${c.w.appEnlazada()}, ` +
    `alarmas=${perdido(c)}`);
  c.w.Date.now = relojC;
};
