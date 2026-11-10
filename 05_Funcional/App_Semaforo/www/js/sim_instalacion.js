// ===== js/sim_instalacion.js =====
// SIMULADOR DE PRACTICA de SPEC_8 8.6 (N-187): contesta SET_CAMARA, SET_PLUMA_RETARDO y CONSULTA_INSTALACION en
// las dos puntas, sobre un estado por poste (p.inst; fabrica NA, NA, 3). Va aparte porque js/simulador.js esta en
// 500 lineas; este lo llama desde sus despachadores. No simula camara ni retardo (igual que plumaCam()): VE:C0D0
// siempre y nunca PLUMA_VETADA. Releido tras guardar como interruptores(): OK solo si quedo lo pedido.
// LITERALES: los de instalacion.cpp. Molde interruptores.cpp: el CMD vuelve con el valor tal como llego (SET_CAMARA:
// C:NC, SET_PLUMA_RETARDO:4) salvo en FORMATO_INVALIDO, que vuelve sin el; el $ERR no lleva VE. tests/dom_simulador.js
// juzga estas tramas contra el C++ y manda el C++.
// El rango del retardo NO se copia: es Instalacion.RETARDO (js/instalacion.js), el unico sitio.
const SimInstalacion = (() => {
  const RANGO = () => (typeof Instalacion !== 'undefined' ? Instalacion : require('./instalacion.js')).RETARDO;
  const estado = (p) => p.inst || (p.inst = { c: 'NA', d: 'NA', r: RANGO().FABRICA });
  const VE = ',VE:C0D0';
  const consulta = (p) => {
    const e = estado(p);
    return '$ACK,CMD:CONSULTA_INSTALACION,RESULT:C_' + e.c + '_D_' + e.d + '_R' + e.r + VE;
  };
  // a: la orden sin "CMD:PIN:<pin>:". Devuelve la trama o null si no es de este modulo.
  function orden(p, a) {
    if (a === 'CONSULTA_INSTALACION') return consulta(p);
    const e = estado(p);
    let cmd, clave, valor;
    const t = a.slice(a.indexOf(':') + 1);
    if (a.indexOf('SET_CAMARA:') === 0) {
      cmd = 'SET_CAMARA';
      const m = /^([CD]):(NA|NC)$/.exec(t);
      if (m) { clave = m[1].toLowerCase(); valor = m[2]; }
    } else if (a.indexOf('SET_PLUMA_RETARDO:') === 0) {
      cmd = 'SET_PLUMA_RETARDO';
      if (/^\d{1,2}$/.test(t)) { clave = 'r'; valor = +t; }   // como el firmware: 1 o 2 cifras
    } else return null;
    if (!clave) return '$ERR,CMD:' + cmd + ',DESC:FORMATO_INVALIDO';
    const c = cmd + ':' + t;
    if (clave === 'r' && (valor < RANGO().MIN || valor > RANGO().MAX)) return '$ERR,CMD:' + c + ',DESC:FUERA_DE_RANGO';
    if (e[clave] === valor) return '$ACK,CMD:' + c + ',RESULT:SIN_CAMBIO' + VE;
    e[clave] = valor;
    return e[clave] === valor ? '$ACK,CMD:' + c + ',RESULT:OK' + VE : '$ERR,CMD:' + c + ',DESC:NO_QUEDO_PUESTO';
  }
  // La consulta sin PIN en el Esclavo (en el Maestro entra por SIN_PIN_M): null si cmd no es ella.
  const sinPin = (p, cmd) => (cmd === 'CMD:CONSULTA_INSTALACION' ? consulta(p) : null);
  return { orden, sinPin };
})();

if (typeof module !== 'undefined' && module.exports) module.exports = globalThis.SimInstalacion = SimInstalacion;
