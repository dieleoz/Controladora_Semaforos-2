// ===== tests/dom_aviso_corte.js =====
// N-170, AVISO DE CORTE DE LUZ. El puente ESP32 manda al conectarse un telefono
// $EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:<nombreCausa()>,... (ESP32_Expansion/src/vigilante.cpp).
// En campo (evidencia/300920261130, 30/09) la app pinto esa trama como
// "Equipo [FIRMWARE]: " vacia: la rama $EVENT solo leia ORIGEN/DETALLE, que esta
// trama no trae. Lo llama test_dom_execution.js, como dom_version.js.
module.exports = async function pruebaAvisoCorte(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const ARRANQUE = 'EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:SUBIDA_DE_TENSION,ARRANQUES:1,PERRO:ARMADO,WDT_MS:2000';
  // La trama exacta de campo, con su checksum de campo: si el xor no diera 34 la
  // prueba no seria la de campo.
  assert(xor(ARRANQUE) === '34', `Corte: la trama de prueba es la de campo (*34): *${xor(ARRANQUE)}`);

  const a = montarAppLimpia();   // ya llego un $STATUS NODE:MAESTRO MODO:AUTO
  const d = a.d;
  const meter = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const eventos = () => Array.from(d.querySelectorAll('#event-feed .event-item'))
                             .map(n => n.textContent.replace(/\s+/g, ' '));
  const cartel = d.getElementById('aviso-corte');
  const txtCartel = () => (cartel ? cartel.textContent.replace(/\s+/g, ' ') : '');

  const antes = eventos().length;
  meter(ARRANQUE);
  const nuevas = eventos().slice(0, eventos().length - antes);
  // (1) Ninguna linea vacia "Equipo [FIRMWARE]: " (el defecto de campo).
  assert(nuevas.length >= 1 && nuevas.every(t => !/Equipo \[FIRMWARE\]:\s*$/.test(t.trim())),
    `Corte: la trama ARRANQUE no deja una linea vacia en Eventos: ${JSON.stringify(nuevas)}`);
  assert(nuevas.some(t => /SIN LUZ/.test(t) && /no se sabe cuando/i.test(t)),
    `Corte: la linea de Eventos dice que se quedo sin luz y que no se sabe cuando: ${JSON.stringify(nuevas)}`);

  // (2) Aviso visible, con poste y modo del $STATUS.
  assert(cartel && !cartel.hidden && /SE QUEDO SIN LUZ/.test(txtCartel()),
    `Corte: sale el cartel visible #aviso-corte: "${txtCartel().slice(0, 120)}"`);
  assert(/POSTE 1/.test(txtCartel()) && /MAESTRO/.test(txtCartel()) && /AUTOM/.test(txtCartel()),
    `Corte: el cartel nombra el poste y el modo del $STATUS: "${txtCartel()}"`);

  // (3) Una vez por conexion: la segunda trama igual no escribe otra linea.
  const n1 = eventos().length;
  meter(ARRANQUE);
  assert(eventos().length === n1, `Corte: la trama repetida en la misma conexion no repite el aviso`);

  // (4) El cartel sigue el modo que llega despues.
  meter('STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:DEGRADADO,ESTADO:R1_R2,T:31,RF:0,RTT:0,BAT:12.9,HORA:14:31:10');
  assert(/DEGRADADO/.test(txtCartel()), `Corte: el cartel pasa al modo nuevo: "${txtCartel()}"`);

  // (5) Entendido lo cierra.
  const ok = d.getElementById('btn-aviso-corte-ok');
  if (ok) ok.click();
  assert(cartel && cartel.hidden, 'Corte: el boton Entendido cierra el cartel');

  // (6) Otra causa (perro): linea legible, sin cartel de corte.
  const b = montarAppLimpia();
  const meterB = (carga) => b.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  meterB('EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:OTRO_PERRO,ARRANQUES:3,PERRO:ARMADO,WDT_MS:2000');
  const lineaB = b.d.querySelector('#event-feed .event-item');
  const tB = lineaB ? lineaB.textContent.replace(/\s+/g, ' ') : '';
  const cartelB = b.d.getElementById('aviso-corte');
  assert(/OTRO_PERRO/.test(tB) && /perro guardian/i.test(tB) && /3/.test(tB) && cartelB && cartelB.hidden,
    `Corte: CAUSA:OTRO_PERRO sale traducida en una linea, sin cartel de corte: "${tB}"`);
};
