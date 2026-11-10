// ===== tests/dom_simulador_rojo.js =====
// FORZAR_ROJO y DEG_T en el simulador de Practica contra lo MEDIDO en el C++ real (10/11, DLL del arnes de dos puntas).
// Sacado de dom_simulador.js por su tope de 500 lineas. CLAUDE.md 9: el caso AMBAR EXIGIA el defecto (Esclavo a ambar a
// los 28 s, modo y ESC: AMBAR); con D-170 se INVIRTIO: de sus cuatro propiedades se conservan dos (antes, las dos en
// ambar; despues, el Maestro en rojo) y se invierten dos (modo MANUAL con ESC:ROJO; el Esclavo sigue en rojo).
// D-173 (10/11): los dos casos AUTO EXIGIAN el modo AUTO tras la orden. De sus cuatro propiedades se conservan tres
// (la luz de antes, el amarillo de 3 s en la via en verde, el rojo en los dos) y se invierte una (modo MANUAL); se
// anaden una foto a los 20 min (varias fases: rojo sostenido) y el caso INTELIGENTE (S3 del arnes degradado).
const path = require('path');
const { relojManual } = require('./dom_simulador.js');
module.exports = async function pruebaSimuladorRojo(montarAppLimpia, assert) {
  const S = require(path.join(__dirname, '..', 'js', 'simulador.js'));
  const T0 = new Date(2026, 9, 15, 10, 0, 0).getTime();
  {
    // FORZAR_ROJO fuera de Degradado = coordinador_forzarRojoTotal(), medido el 10/11 con las dos DLL del arnes de dos
    // puntas (C++ real): el verde cierra por AMARILLO_SEG (D-45); en AMBAR (D-170, R1 de orquestador_deg_fin.inc), en
    // AUTO e INTELIGENTE (D-173, bloque S) el Maestro pasa a MANUAL y las dos quedan en rojo fijo, tambien a 20 min.
    const foto = (escV, ambar, modo = 'AUTO') => {
      const r = relojManual(T0), sim = S.crear({ reloj: r, postes: [{ nodo: 'MAESTRO', serie: 'M', modo,
        pareja: 'A' }, { nodo: 'ESCLAVO', serie: 'E', pareja: 'A' }] }), [m, e] = sim.postes;
      const ver = () => { const s = sim.status(m).replace(/\*..$/, '').split(','); return [3, 4, 10].map(i => s[i])
        .concat('E=' + sim.status(e).split(',')[4].slice(7)).join(' ').replace(/(MODO|ESTADO|ESC):/g, ''); };
      r.avanzar(escV ? 200000 : 5000);
      if (ambar) sim.recibir(m, 'CMD:PIN:' + S.C.PIN + ':SET_MODO:AMBAR\r\n');
      const out = [ver()]; sim.recibir(m, 'CMD:FORZAR_ROJO\r\n');
      [1000, 2200, 25000, 1200000].forEach(ms => { r.avanzar(ms); out.push(ver()); });
      return out.join(' > ');
    };
    const fijo = ' > MANUAL ROJO ROJO E=ROJO > MANUAL ROJO ROJO E=ROJO > MANUAL ROJO ROJO E=ROJO';
    const casos = [[foto(false, false), 'AUTO VERDE ROJO E=ROJO > MANUAL AMARILLO ROJO E=ROJO' + fijo],
                   [foto(true, false), 'AUTO ROJO VERDE E=VERDE > MANUAL ROJO ROJO E=AMARILLO' + fijo],
                   [foto(false, false, 'INTELIGENTE'), 'INTELIGENTE VERDE ROJO E=ROJO > MANUAL AMARILLO ROJO E=ROJO' +
                    fijo],
                   [foto(false, true), 'AMBAR FALLO COM AMBAR E=FALLO COM > MANUAL ROJO ROJO E=ROJO' + fijo]];
    // D-171: sin radio, ESC:? solo pasados SFTY6_SILENCIO_MS sin respuesta (coordinador_estadoEsclavo); antes el ultimo
    const r5 = relojManual(T0), s5 = S.crear({ reloj: r5, postes: [{ nodo: 'MAESTRO', serie: 'M', modo: 'AUTO',
      pareja: 'A' }, { nodo: 'ESCLAVO', serie: 'E', pareja: 'A' }] });
    const esc5 = () => s5.status(s5.postes[0]).split(',')[10];
    r5.avanzar(200000); const e5 = [esc5()]; s5.postes[0].radio = false;
    [20000, 9000].forEach(ms => { r5.avanzar(ms); e5.push(esc5()); });
    casos.push([e5.join(' '), 'ESC:VERDE ESC:VERDE ESC:?']);
    // DEG_T aceptado: modo_degradado_entrarTestigo() de las dos puntas fuerza el rojo y espera la hora de inicio.
    const r4 = relojManual(T0), s4 = S.crear({ reloj: r4, postes: [{ nodo: 'MAESTRO', serie: 'M', modo: 'MENU',
      radio: false }, { nodo: 'ESCLAVO', serie: 'E', modo: 'SUBORDINADO', radio: false }] }), dt = [];
    casos.push([s4.status(s4.postes[0]).split(',')[10], 'ESC:?']);   // D-171: MENU sin radio no sabe el color del otro
    s4.postes.forEach(p => s4.recibir(p, 'CMD:PIN:'+ S.C.PIN + ':SET_MODO:DEG_T:10:00:00,10:20:00,300,40\r\n'));
    [1000, 5000, 1190000].forEach(ms => { r4.avanzar(ms); s4.postes.forEach(p => dt.push(s4.status(p).split(',')[4])); });
    casos.push([dt.join(' '), Array(6).fill('ESTADO:ROJO').join(' ')]);
    const malos = casos.filter(([v, q]) => v !== q).map(([v]) => v);
    assert(malos.length === 0, `Simulador: FORZAR_ROJO como el C++ (dos puntas, 10/11): el verde cierra por 3 s de ` +
      `amarillo en su punta; en AMBAR, AUTO e INTELIGENTE pasa a MANUAL con las dos en rojo, tambien a los 28 s y a ` +
      `los 20 min (D-170, D-173); sin radio ESC:? a los 28 s, no antes (D-171); DEG_T aceptado ` +
      `deja las dos puntas en rojo hasta la hora de inicio ` +
      `(${malos.join(' || ') || 'OK'})`);
  }
};
