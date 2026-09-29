// ===== tests/dom_acuse_con_argumentos.js =====
// EL ACUSE DEL TESTIGO QUEDABA SUELTO (29/09). Cinta de campo
// evidencia/2026-09-29_campo_testigo/IOTVIAL_4D2007_20260929-183445.txt: sale
// "SET_MODO:DEG_T:18:26:06,18:29:33,180,30", el equipo contesta
// "$ACK,CMD:SET_MODO:DEG_T,RESULT:OK" y el diario lo pintaba como RESPUESTA SUELTA
// "puede venir de otro telefono": un origen falso. El CMD del acuse es la orden sin
// sus argumentos, y la cabecera de la orden lleva ella misma un ':'.
// Lo llama test_dom_execution.js, como dom_version.js.
module.exports = async function pruebaAcuseConArgumentos(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const D = a.w.DiarioOrdenes;
  D.limpiar();
  // La orden y el acuse EXACTOS de la cinta; el acuse entra por el parser real.
  const orden = 'SET_MODO:DEG_T:18:26:06,18:29:33,180,30';
  D.anotarOrden(orden, 'CMD:PIN:1234:' + orden + '\r\n', Date.now() - 100);
  const carga = 'ACK,CMD:SET_MODO:DEG_T,RESULT:OK';
  a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const r = D.todas().filter(x => x.clase === 'ORDEN' && x.orden === orden).pop();
  const sueltas = D.todas().filter(x => x.clase === 'RESPUESTA_SUELTA');
  assert(!!r && !!r.respuesta && r.respuesta.atribucion === 'POR_CABECERA' && sueltas.length === 0,
    `Testigo: el $ACK,CMD:SET_MODO:DEG_T se atribuye a su orden (POR_CABECERA), no queda suelto: ` +
    `${r && r.respuesta ? r.respuesta.atribucion : '(sin respuesta)'}, sueltas=${sueltas.length}`);
  // Controles negativos: la cabecera se corta en un ':' de la orden, no en cualquier letra.
  assert(D.casa('SET_MODO:DEG', orden) === null && D.casa('SET_MODO:DEG_T', 'SET_MODO:AUTO') === null &&
         D.casa('SET_MODO:DEG_T', 'SET_MODO:DEG_T') === 'EXACTA' &&
         D.casa('SET_TIEMPOS', 'SET_TIEMPOS:12,10,5') === 'POR_CABECERA' &&
         D.casa('SET_RTC', 'SET_RTC:2026-09-29,18:26:06') === 'POR_CABECERA' &&
         D.casa('CAMBIAR_TURNO', 'MANUAL:CAMBIAR_TURNO') === 'POR_ALIAS' &&
         D.casa('SET_MODO:AUTO', 'MANUAL:CAMBIAR_TURNO') === null,
    'Testigo: casa() no se vuelve "casi todo casa" (prefijo solo hasta un ":", alias y exactas intactos)');
};
