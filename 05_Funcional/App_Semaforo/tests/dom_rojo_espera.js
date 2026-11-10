// ===== tests/dom_rojo_espera.js =====
// D-175 (SPEC_8 8.4): el ROJO DE ESPERA. Con el Maestro en MENU la pluma que estaba arriba se queda arriba: la app no
// puede decir "no baja mientras haya alguien debajo" (D-13), que es un veto que no hay. Y el simulador de Practica
// sigue al firmware: en MENU la pluma no baja (solo retiene: la que estaba abajo no sube) y ROJO TOTAL la baja.
// Lo llama test_dom_execution.js.
const path = require('path');
const { relojManual } = require('./dom_simulador.js');
module.exports = async function pruebaRojoEspera(montarAppLimpia, assert) {
  const xor = (s) => { let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
                       return c.toString(16).toUpperCase().padStart(2, '0'); };
  const a = montarAppLimpia();
  const entra = (carga) => a.w._btSubscribeCb(`$${carga}*${xor(carga)}\n`);
  const st = (modo, pl) => entra(`STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:${modo},ESTADO:ROJO,T:--,RF:97,RTT:70,` +
    `BAT:12.9,HORA:14:31:00,ESC:ROJO,PLUMA:${pl}`);
  const frase = () => a.d.getElementById('pluma-estado').textContent;

  st('MENU', 'ARRIBA');
  const enMenu = frase();
  assert(/rojo de espera: sigue arriba hasta que arranque un modo/.test(enMenu) && !/alguien debajo/.test(enMenu),
    `RojoEspera (D-175): MODO:MENU + ROJO + PLUMA:ARRIBA dice que es el rojo de espera, no un veto: "${enMenu}"`);
  st('AUTO', 'ARRIBA');   // control: en un rojo de ciclo la frase sigue siendo la de D-13
  assert(/no baja mientras haya alguien debajo/.test(frase()),
    `RojoEspera: control, MODO:AUTO + ROJO + PLUMA:ARRIBA sigue diciendo el veto de D-13: "${frase()}"`);

  // El simulador: Maestro y Esclavo emparejados en AUTO; MENU con la pluma de un poste arriba (su verde).
  const S = require(path.join(__dirname, '..', 'js', 'simulador.js'));
  const T0 = new Date(2026, 10, 10, 10, 0, 0).getTime();
  const caso = (quienVerde) => {
    const r = relojManual(T0), sim = S.crear({ reloj: r, postes: [{ nodo: 'MAESTRO', serie: 'M', modo: 'AUTO',
      pareja: 'A' }, { nodo: 'ESCLAVO', serie: 'E', pareja: 'A' }] }), [m, e] = sim.postes;
    const v = (p) => { const s = sim.status(p); return /ESTADO:([^,]+)/.exec(s)[1] + '/' + /PLUMA:(\w+)/.exec(s)[1]; };
    const verde = quienVerde === 'M' ? m : e;
    for (let n = 0; n < 600 && !/^VERDE/.test(v(verde)); n++) { v(m); v(e); r.avanzar(1000); }
    const antes = v(m) + ' ' + v(e);
    sim.recibir(m, 'CMD:SET_MODO:MENU\r\n');
    r.avanzar(1000); v(m); v(e); r.avanzar(30000);
    const menu = v(m) + ' ' + v(e);
    sim.recibir(m, 'CMD:FORZAR_ROJO\r\n');
    r.avanzar(5000);
    const rojoTotal = v(m) + ' ' + v(e);
    sim.parar();
    return [antes, menu, rojoTotal].join(' > ');
  };
  const cM = caso('M'), cE = caso('E');
  assert(cM === 'VERDE/ARRIBA ROJO/ABAJO > ROJO/ARRIBA ROJO/ABAJO > ROJO/ABAJO ROJO/ABAJO',
    `RojoEspera (D-175): simulador, MENU desde el verde del Maestro: su pluma sigue arriba a los 30 s, la del ` +
    `Esclavo (abajo) no sube, y ROJO TOTAL baja las dos: ${cM}`);
  assert(cE === 'ROJO/ABAJO VERDE/ARRIBA > ROJO/ABAJO ROJO/ARRIBA > ROJO/ABAJO ROJO/ABAJO',
    `RojoEspera (D-175): simulador, MENU desde el verde del Esclavo: su pluma sigue arriba a los 30 s y ROJO ` +
    `TOTAL la baja: ${cE}`);
};
