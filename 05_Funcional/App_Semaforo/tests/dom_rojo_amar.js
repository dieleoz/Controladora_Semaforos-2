// ===== tests/dom_rojo_amar.js =====
// D-53: ESTADO:ROJO+AMAR (rojo y amarillo a la vez antes de cada verde). El literal es el que
// fija el diseno de D-53 (punto 9) para semaforo_nombreEstado(). Lo llama test_dom_execution.js.
module.exports = async function pruebaRojoAmar(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const d = a.d;
  const entra = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const st = (nodo, est, extra = '') => entra(`STATUS,NODE:${nodo},SERIE:SEM-M-01,MODO:AUTO,ESTADO:${est},T:2,` +
    `RF:97,RTT:70,BAT:12.9,HORA:14:31:00${extra}`);
  const on = (id) => d.getElementById(id).classList.contains('active');
  const txt = (id) => d.getElementById(id).textContent;

  st('MAESTRO', 'ROJO+AMAR', ',ESC:ROJO,PLUMA:ABAJO');
  assert(on('s1-red') && on('s1-amber') && !on('s1-green'),
    `RojoAmar: Maestro en ROJO+AMAR enciende rojo Y amarillo, no el verde`);
  assert(txt('s1-text') === 'ROJO Y AMARILLO' && /Rojo y amarillo: va a abrir el verde/.test(txt('phase-desc')) &&
         !/NO RECONOCIDO|no reconocido/.test(txt('s1-text') + txt('phase-desc')),
    `RojoAmar: texto y frase propios, sin "ESTADO NO RECONOCIDO": "${txt('s1-text')}" / "${txt('phase-desc')}"`);
  assert(!/\d\s*s\b/.test(txt('phase-desc').split(' · ')[0]),
    `RojoAmar: la frase no lleva cifra de segundos (CLAUDE.md 14): "${txt('phase-desc').split(' · ')[0]}"`);
  // El numero del anillo (div); el arco es SVG y su className no se asigna (defecto aparte, reportado).
  assert(/amber/.test(d.getElementById('cd-num').className),
    `RojoAmar: el numero del anillo va en ambar: ${d.getElementById('cd-num').className}`);

  // Control: ROJO a secas enciende SOLO el rojo (si no, lo de arriba aprobaria siempre).
  st('MAESTRO', 'ROJO', ',ESC:ROJO,PLUMA:ABAJO');
  assert(on('s1-red') && !on('s1-amber') && !on('s1-green'), 'RojoAmar: control, ROJO apaga el amarillo');

  // La pluma: arriba durante R+A no es "acompana al verde" (sube con el VERDE real).
  st('MAESTRO', 'ROJO+AMAR', ',ESC:ROJO,PLUMA:ARRIBA');
  const pl = txt('pluma-estado');
  assert(/NO es aver/.test(pl) && !/acompa/.test(pl),
    `RojoAmar: pluma arriba en R+A se lee como en rojo, no como verde: "${pl}"`);

  // El Esclavo pinta su propia columna igual.
  st('ESCLAVO', 'ROJO+AMAR');
  assert(on('s2-red') && on('s2-amber') && !on('s2-green') && txt('s2-text') === 'ROJO Y AMARILLO',
    `RojoAmar: el Esclavo enciende rojo y amarillo en su columna: "${txt('s2-text')}"`);
};
