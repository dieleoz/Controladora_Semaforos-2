// ===== tests/dom_version.js =====
// LA VERSION SE VE EN PANTALLA (29/09). En banco el funcional pulso "Consultar version"
// y solo le salio "Version contestada - vea el campo FW": el hash llegaba en la trama
// pero ninguna linea de Eventos lo mostraba, y la del puente salia vacia
// ("Equipo [FIRMWARE]: "). Lo llama test_dom_execution.js, como dom_exportar.js.
module.exports = async function pruebaVersionVisible(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const meter = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const eventos = () => Array.from(a.d.querySelectorAll('.event-item'))
                             .map(n => n.textContent.replace(/\s+/g, ' ')).join(' || ');
  meter('ACK,CMD:VERSION,RESULT:OK,NODE:MAESTRO,FW:abc1234');
  assert(/abc1234/.test(eventos()),
    'Version: el hash del CONTROLADOR (FW del $ACK) sale escrito en Eventos');
  meter('EVENT,NODE:PUENTE,EVT:VERSION,FW:def5678');
  assert(/def5678/.test(eventos()),
    'Version: el hash del PUENTE (FW del $EVENT) sale escrito en Eventos');

  // LA LINEA VACIA DE CAMPO: la trama EXACTA de la cinta (evidencia/011020260934/IOTVIAL_*.txt,
  // *7F) pintaba ademas "Equipo [FIRMWARE]: " sin nada detras (Log_Semaforos_*.csv).
  const SELLO = 'EVENT,NODE:PUENTE,EVT:VERSION,FW:6bd1e4f';
  assert(xor(SELLO) === '7F', `Version: la trama de prueba es la de campo (*7F): *${xor(SELLO)}`);
  const b = montarAppLimpia();
  const meterB = (carga) => b.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const lineasB = () => Array.from(b.d.querySelectorAll('#event-feed .event-item'))
                             .map(n => n.textContent.replace(/\s+/g, ' ').trim());
  const vacia = /Equipo \[[^\]]*\]:\s*(-|->|$)/;
  meterB(SELLO);
  assert(lineasB().some(t => /Version cargada - Puente \(ESP32\): FW 6bd1e4f/.test(t)) &&
         !lineasB().some(t => vacia.test(t)),
    `Version: el sello del puente sale UNA vez y sin linea "Equipo [..]: " vacia: ${JSON.stringify(lineasB().slice(0, 3))}`);
  // Ninguna rama generica pinta "Equipo [X]: " vacio: sin DETALLE, los campos crudos.
  meterB('EVENT,NODE:ESCLAVO,EVT:NUEVO_SIN_TRADUCIR,CUENTA:7');
  assert(!vacia.test(lineasB()[0]) && /EVT:NUEVO_SIN_TRADUCIR/.test(lineasB()[0]) && /CUENTA:7/.test(lineasB()[0]),
    `Version: un $EVENT sin DETALLE sale con sus campos crudos: "${lineasB()[0]}"`);
};
