// ===== tests/dom_simulador_rojo.js =====
// FORZAR_ROJO y DEG_T en el simulador de Practica contra lo MEDIDO en el C++ real (10/11, DLL del arnes de dos puntas).
// Sacado de dom_simulador.js por su tope de 500 lineas. OJO (CLAUDE.md 9): el caso AMBAR describe el firmware de HOY,
// que deja volver al Esclavo a ambar a los 28 s; el arreglo decidido el 10/11 (rojo fijo sostenido en los dos) lo INVIERTE.
const path = require('path');
const { relojManual } = require('./dom_simulador.js');
module.exports = async function pruebaSimuladorRojo(montarAppLimpia, assert) {
  const S = require(path.join(__dirname, '..', 'js', 'simulador.js'));
  const T0 = new Date(2026, 9, 15, 10, 0, 0).getTime();
  {
    // FORZAR_ROJO fuera de Degradado = coordinador_forzarRojoTotal(), medido el 10/11 con las dos DLL del arnes de dos
    // puntas (C++ real): el verde cierra por AMARILLO_SEG (D-45); en AMBAR el Maestro pasa a rojo fijo (modo y ESC:
    // AMBAR quedan) y el Esclavo a rojo hasta que la orfandad (SFTY6_SILENCIO_MS) lo devuelve a su ambar.
    const foto = (escV, ambar) => {
      const r = relojManual(T0), sim = S.crear({ reloj: r, postes: [{ nodo: 'MAESTRO', serie: 'M', modo: 'AUTO',
        pareja: 'A' }, { nodo: 'ESCLAVO', serie: 'E', pareja: 'A' }] }), [m, e] = sim.postes;
      const ver = () => { const s = sim.status(m).replace(/\*..$/, '').split(','); return [3, 4, 10].map(i => s[i])
        .concat('E=' + sim.status(e).split(',')[4].slice(7)).join(' ').replace(/(MODO|ESTADO|ESC):/g, ''); };
      r.avanzar(escV ? 200000 : 5000);
      if (ambar) sim.recibir(m, 'CMD:PIN:' + S.C.PIN + ':SET_MODO:AMBAR\r\n');
      const out = [ver()]; sim.recibir(m, 'CMD:FORZAR_ROJO\r\n');
      [1000, 2200, 25000].forEach(ms => { r.avanzar(ms); out.push(ver()); });
      return out.join(' > ');
    };
    const casos = [[foto(false, false), 'AUTO VERDE ROJO E=ROJO > AUTO AMARILLO ROJO E=ROJO > AUTO ROJO ROJO E=ROJO' +
                    ' > AUTO ROJO ROJO E=ROJO'],
                   [foto(true, false), 'AUTO ROJO VERDE E=VERDE > AUTO ROJO ROJO E=AMARILLO > AUTO ROJO ROJO E=ROJO' +
                    ' > AUTO ROJO ROJO E=ROJO'],
                   [foto(false, true), 'AMBAR FALLO COM AMBAR E=FALLO COM > AMBAR ROJO AMBAR E=ROJO > AMBAR ROJO AMBAR' +
                    ' E=ROJO > AMBAR ROJO AMBAR E=FALLO COM']];
    // DEG_T aceptado: modo_degradado_entrarTestigo() de las dos puntas fuerza el rojo y espera la hora de inicio.
    const r4 = relojManual(T0), s4 = S.crear({ reloj: r4, postes: [{ nodo: 'MAESTRO', serie: 'M', modo: 'MENU',
      radio: false }, { nodo: 'ESCLAVO', serie: 'E', modo: 'SUBORDINADO', radio: false }] }), dt = [];
    s4.postes.forEach(p => s4.recibir(p, 'CMD:PIN:' + S.C.PIN + ':SET_MODO:DEG_T:10:00:00,10:20:00,300,40\r\n'));
    [1000, 5000, 1190000].forEach(ms => { r4.avanzar(ms); s4.postes.forEach(p => dt.push(s4.status(p).split(',')[4])); });
    casos.push([dt.join(' '), Array(6).fill('ESTADO:ROJO').join(' ')]);
    const malos = casos.filter(([v, q]) => v !== q).map(([v]) => v);
    assert(malos.length === 0, `Simulador: FORZAR_ROJO como el C++ (dos puntas, 10/11): el verde cierra por 3 s de ` +
      `amarillo en su punta; en AMBAR el Maestro queda en rojo y el Esclavo vuelve a ambar a los 28 s; DEG_T aceptado ` +
      `deja las dos puntas en rojo hasta la hora de inicio ` +
      `(${malos.join(' || ') || 'OK'})`);
  }
};
