// ===== js/sim_interruptores.js =====
// SIMULADOR DE PRACTICA de SPEC_8 6 y 9 (D-94, D-166, D-169): interruptores_orden() de {Maestro,Esclavo}/src/
// interruptores.cpp, identico en las dos puntas y local al poste. Va aparte porque js/simulador.js esta en 500 lineas;
// este lo llama desde sus despachadores. Estado por poste: p.vetoC, p.vetoD (inhibida), p.mala1, p.mala2 (true =
// BAJA), p.barreraFuera; RESPALDO_INVALIDO con p.respaldoPerdido (D-148). No simula camaras: cada una '?' (sin
// flanco), asi que CAM: es '?' o NOVETO, igual que plumaCam() antes. LITERALES: los de interruptores.cpp;
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
  const consulta = (p) => '$ACK,CMD:CONSULTA_VETO,RESULT:C' + (p.vetoC ? '0' : '1') + '?_D' + (p.vetoD ? '0' : '1') +
                          '?_M1' + (p.mala1 ? 'B' : 'A') + '_M2' + (p.mala2 ? 'B' : 'A');
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
  // D-151/D-166: sin camaras simuladas la peor de las no inhibidas es '?': NOVETO si hay alguna inhibida.
  const cam = (p) => (p.vetoC || p.vetoD ? 'NOVETO' : '?');
  // La consulta sin PIN en el Esclavo (en el Maestro entra por SIN_PIN_M): null si cmd no es ella.
  const sinPin = (p, cmd) => (cmd === 'CMD:CONSULTA_VETO' ? consulta(p) : null);
  return { orden, cam, sinPin };
})();

if (typeof module !== 'undefined' && module.exports) module.exports = globalThis.SimInterruptores = SimInterruptores;
