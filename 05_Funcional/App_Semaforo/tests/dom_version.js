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
};
