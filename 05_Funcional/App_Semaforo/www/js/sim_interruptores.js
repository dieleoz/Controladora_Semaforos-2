// ===== js/sim_interruptores.js =====
// SIMULADOR DE PRACTICA de SPEC_8 6 y 9 (D-94, D-166, D-169): interruptores_orden() de {Maestro,Esclavo}/src/
// interruptores.cpp, identico en las dos puntas y local al poste. Va aparte porque js/simulador.js esta en 500 lineas;
// este lo llama desde sus despachadores. Estado por poste: p.vetoC, p.vetoD (inhibida), p.mala1, p.mala2 (true =
// BAJA), p.barreraFuera; RESPALDO_INVALIDO con p.respaldoPerdido (D-148); p.camC, p.camD: la letra de camara_letra()
// ('O' OK, '?' sin flanco, 'C' CIEGA, 'P' PEGADA). Sin letra puesta, '?': el firmware antes del primer flanco y el
// CAM:? de las cintas de campo; la flota de js/practica.js pone las suyas. De esas letras salen CONSULTA_VETO y CAM:
// con la regla de camara_estado() + interruptores_camNoVeto(). NO MODELA: flancos,
// silencio ni nivel (nada pone sola una 'C' o una 'P' ni la recupera), las alarmas CAM_PEGADA/CAM_CIEGA, ni el efecto
// de una camara mala sobre la pluma (D-169, MALAx): la letra solo se lee. LITERALES: los de interruptores.cpp;
// tests/dom_simulador.js juzga estas tramas contra el C++ y manda el C++.
const SimInterruptores = (() => {
  const SUJETO = { 'SET_VETO_CAMARA': ['ON', 'OFF'], 'SET_CAMARA_MALA': ['ARRIBA', 'BAJA'],
                   'SET_BARRERA_SERVICIO': ['ON', 'OFF'] };
  // Las claves del estado que la orden mira, o null si el valor no casa (FORMATO_INVALIDO).
  function claves(cmd, v) {
    if (cmd === 'SET_BARRERA_SERVICIO') return { claves: ['barreraFuera'], v };
    if (cmd === 'SET_VETO_CAMARA') {
      const m = /^([CD]):(.*)$/.exec(v);
      return m ? { claves: [m[1] === 'C' ? 'vetoC' : 'vetoD'], v: m[2] } : { claves: ['vetoC', 'vetoD'], v };
    }
    const m = /^([12]):(.*)$/.exec(v);
    return m ? { claves: ['mala' + m[1]], v: m[2] } : null;
  }
  // camara_letra(i), sin mirar la inhibicion (botones_vigilante.inc).
  const letra = (p, k) => (['O', '?', 'C', 'P'].indexOf(p['cam' + k]) >= 0 ? p['cam' + k] : '?');
  const consulta = (p) => '$ACK,CMD:CONSULTA_VETO,RESULT:C' + (p.vetoC ? '0' : '1') + letra(p, 'C') + '_D' +
                          (p.vetoD ? '0' : '1') + letra(p, 'D') + '_M1' + (p.mala1 ? 'B' : 'A') + '_M2' +
                          (p.mala2 ? 'B' : 'A');
  // a: la orden sin "CMD:PIN:<pin>:". Devuelve la trama o null si no es de este modulo.
  function orden(p, a) {
    if (a === 'CONSULTA_VETO') return consulta(p);
    const cmd = Object.keys(SUJETO).find(k => a.indexOf(k + ':') === 0);
    if (!cmd) return null;
    const valor = a.slice(cmd.length + 1), s = claves(cmd, valor);
    const [fabrica, puesto] = SUJETO[cmd];
    if (!s || (s.v !== fabrica && s.v !== puesto)) return '$ERR,CMD:' + cmd + ',DESC:FORMATO_INVALIDO';
    const pon = s.v === puesto, enEstado = () => s.claves.every(k => !!p[k] === pon), c = cmd + ':' + valor;
    if (enEstado()) return '$ACK,CMD:' + c + ',RESULT:SIN_CAMBIO';
    if (pon && p.respaldoPerdido) return '$ERR,CMD:' + c + ',DESC:RESPALDO_INVALIDO';
    s.claves.forEach(k => { p[k] = pon; });
    return enEstado() ? '$ACK,CMD:' + c + ',RESULT:OK' : '$ERR,CMD:' + c + ',DESC:NO_QUEDO_PUESTO';
  }
  // camara_estado(): la peor de las NO inhibidas que se pueden juzgar ('?' no cuenta); ninguna, '?'. Y D-151/D-166
  // (interruptores_camNoVeto): con alguna inhibida y la peor OK o '?', NOVETO.
  function cam(p) {
    const vivas = ['C', 'D'].filter(k => !p['veto' + k]).map(k => letra(p, k)).filter(l => l !== '?');
    const e = vivas.indexOf('P') >= 0 ? 'PEGADA' : vivas.indexOf('C') >= 0 ? 'CIEGA' : vivas.length ? 'OK' : '?';
    return (p.vetoC || p.vetoD) && (e === 'OK' || e === '?') ? 'NOVETO' : e;
  }
  // La consulta sin PIN en el Esclavo (en el Maestro entra por SIN_PIN_M): null si cmd no es ella.
  const sinPin = (p, cmd) => (cmd === 'CMD:CONSULTA_VETO' ? consulta(p) : null);
  return { orden, cam, sinPin };
})();

if (typeof module !== 'undefined' && module.exports) module.exports = globalThis.SimInterruptores = SimInterruptores;
