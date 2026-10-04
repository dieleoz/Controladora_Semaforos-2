// ===== js/textos_spec4.js =====
// Textos de los $ACK y $ERR de SPEC_4 §3.1/§3.2 que caian en el generico "ACEPTADA (...)" o en "motivo sin traducir",
// y el texto de la emergencia del Maestro segun el modo. Molde de ACK_TEXTO / ERR_TEXTO de app.js (que los consulta
// DESPUES de esta tabla): {tono, texto, toast}. Para quien esta de pie en la calzada: cortos, solo lo que el equipo dijo,
// y ninguno promete el OTRO poste. ERR va por 'CMD|DESC', como ERR_TEXTO. Sin cifras del firmware (CLAUDE.md 14).
const TextosSpec4 = {
  ACK: {
    // SPEC_4 §3.1: OK incondicional, la rama entra por el todo-rojo; no hay rechazo posible.
    'SET_MODO:AUTO|OK': { tono: 'cyan',
      texto: 'Equipo: orden AUTOMATICO aceptada en ESTE poste. Entra por un todo-rojo y despues cicla: ' +
             'compruebe con los ojos que las cabezas alternan.',
      toast: 'Automatico aceptado: entra por todo-rojo' },
    'SET_MODO:MANUAL|OK': { tono: 'cyan',
      texto: 'Equipo: orden MANUAL aceptada en ESTE poste. Entra por el todo-rojo y espera: el paso se da ' +
             'con DAR PASO.',
      toast: 'Manual aceptado: todo-rojo hasta DAR PASO' },
    // SPEC_4 §3.1 y §7: ALCANCE e INTELIGENTE contestan OK sin condicion; la luz es lo que dice si entro.
    'SET_MODO:ALCANCE|OK': { tono: 'cyan',
      texto: 'Equipo: orden PRUEBA DE ALCANCE aceptada. El equipo contesta OK siempre: mire la luz, que ' +
             'debe quedar en rojo fijo.',
      toast: 'Alcance aceptado: compruebe el rojo fijo' },
    'SET_MODO:INTELIGENTE|OK': { tono: 'cyan',
      texto: 'Equipo: orden INTELIGENTE aceptada. El equipo contesta OK siempre: el modo se ve en el rotulo ' +
             'del modo y en la luz, no en este aviso.',
      toast: 'Inteligente aceptado: compruebe el modo' },
    'TEST_LEDS|STARTING_6S': { tono: 'cyan',
      texto: 'Equipo: prueba de LEDs ARMADA, dura unos 6 s. Mire las lamparas mientras corre.',
      toast: 'Prueba de LEDs en marcha' },
    // SPEC_6 PARTE C: REINICIAR_RELOJ borra hora, ciclo y autorizacion; el cristal arranca y cuenta, falta la hora.
    'REINICIAR_RELOJ|CRISTAL_OK_PONGA_LA_HORA': { tono: 'amber',
      texto: 'Equipo: el reloj ARRANCO y cuenta, pero SIN HORA: PONGA LA HORA ahora (pestana Tecnico, ' +
             'Sincronizar). El reinicio borro hora, ciclo y autorizacion.',
      toast: 'Reloj en marcha: falta poner la hora' },
    'DEMANDA|REGISTRADA': { tono: 'cyan',
      texto: 'Equipo: demanda REGISTRADA. Queda pedida; el paso llega cuando el modo la atienda, no al instante.',
      toast: 'Demanda registrada' },
  },

  ERR: {
    'CAMBIAR_TURNO|MODO_SIN_CICLO_SALGA_PRIMERO': {
      texto: 'Equipo: DAR PASO rechazado, no se cambio nada: el modo actual no mueve el ciclo. Pase a ' +
             'AUTOMATICO o MANUAL y repita.',
      toast: 'Dar paso rechazado: el modo no tiene ciclo' },
    'TEST_LEDS|SIN_ENLACE_AMBAR_NO_SE_PRUEBA': {
      texto: 'Equipo: prueba de LEDs NO armada. El cruce esta en ambar por falta de enlace y en ese ' +
             'estado no se prueba. Reponga el enlace entre postes y repita.',
      toast: 'Prueba de LEDs no armada: sin enlace, ambar' },
    'TEST_LEDS|ESPERANDO_ROJO_DEL_ESCLAVO': {
      texto: 'Equipo: prueba de LEDs NO armada: el Esclavo aun no confirmo su rojo, y la secuencia ' +
             'enciende verde. Espere a que confirme y repita.',
      toast: 'Prueba de LEDs no armada: falta el rojo del Esclavo' },
    'TEST_LEDS|EN_SERVICIO_PASE_A_MENU': {
      texto: 'Equipo: prueba de LEDs NO armada: el cruce esta en servicio y la prueba solo va fuera de ' +
             'servicio. Pase a MENU y repita.',
      toast: 'Prueba de LEDs no armada: pase a MENU' },
  },

  // SPEC_4 §3.ter.ter 1 y SPEC_2 §7.quater (b): en Degradado el rojo fijo es solo de ESTE poste; el otro sigue
  // alternando. Fuera de Degradado vale el texto de D-45 (rojo fijo en las dos vias). `punta` es null si no se sabe.
  emergenciaMaestro(punta, degradado) {
    if (degradado) {
      return { sub: punta ? 'Rojo fijo solo en este poste' : 'POSTE 1 · rojo fijo solo en este poste',
        hint: 'POSTE 1 en Degradado: la parada de emergencia deja en ROJO FIJO solo ESTE poste; el otro ' +
              'sigue alternando por su reloj.',
        orden: 'ALERTA: orden ROJO TOTAL DE EMERGENCIA enviada al MAESTRO. En Degradado, si el equipo la ' +
               'acepta deja en rojo fijo SOLO este poste; el otro sigue alternando.' };
    }
    return { sub: punta ? 'Ambas vías en rojo fijo' : 'POSTE 1 · ambas vías en rojo fijo',
      hint: 'POSTE 1: la parada de emergencia deja las dos vías en ROJO FIJO y detiene el tráfico.',
      orden: 'ALERTA: orden ROJO TOTAL DE EMERGENCIA enviada al MAESTRO. Si el equipo la acepta deja las ' +
             'dos vias en rojo fijo.' };
  },
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = TextosSpec4;
}
