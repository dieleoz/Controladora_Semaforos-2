// ===== js/sim_puente.js =====
// SIMULADOR DE PRACTICA DEL PUENTE (D-177, SPEC_4 9.10 (c)): solo el SET_RTC que el testigo y la salida mandan antes
// de su orden. Lo atiende el puente de cada poste (01_Firmware/ESP32_Expansion/src/despachador.cpp), con o sin PIN
// (las dos formas casan strstr(linea, "SET_RTC:")): $ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,FECHA:..,HORA:.. con lo
// escrito y releido, o $ERR,NODE:PUENTE,CMD:SET_RTC,DESC:FORMATO_INVALIDO si no son seis numeros. Deja el poste en
// hora (p.enHora), como el puente deja el reloj del STM32. NO MODELA: HORA_PUESTA_SIN_PROPAGAR, el reloj ausente ni
// LEER_RTC y LOG, que siguen sin contestarse. Va aparte porque js/simulador.js esta en 500 lineas.
const SimPuente = (() => {
  const dd = (n, w) => String(n).padStart(w || 2, '0');
  // cmd: la linea sin fin de linea. Devuelve true si era del puente (y ya contesto por tx), false si no.
  function orden(p, cmd, tx) {
    const m = /^CMD:(?:PIN:\d+:)?SET_RTC:(.*)$/.exec(String(cmd || ''));
    if (!m) return false;
    const f = /^(\d+)-(\d+)-(\d+),(\d+):(\d+):(\d+)$/.exec(m[1]);
    if (!f) { tx('$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:FORMATO_INVALIDO'); return true; }
    p.enHora = true;
    tx('$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,FECHA:' + dd(f[1], 4) + '-' + dd(f[2]) + '-' + dd(f[3]) +
       ',HORA:' + dd(f[4]) + ':' + dd(f[5]) + ':' + dd(f[6]));
    return true;
  }
  return { orden };
})();

if (typeof module !== 'undefined' && module.exports) module.exports = globalThis.SimPuente = SimPuente;
