// ===== tests/dom_tres_modos.js =====
// D-177 (SPEC_4 9.16, 3.ter, 3.ter.ter, 9.10): tres modos (Operario, Degradado, Tecnico), el asistente de radio caida,
// Averias con selectores y la practica sin clave. Los valores esperados salen de la decision (claves 0000 y 1234), de
// js/testigo.js y js/deg_fin.js (20 min, 3 min, 30 s, que citan SPEC_4 3.ter y el firmware), del literal del puente
// (ESP32_Expansion/src/despachador.cpp) y de las condiciones del arquitecto (cond. 1 a 5). No corre sola: la llama
// test_dom_execution.js con SU montarAppLimpia y SU assert. Lo que no mide: telefono, sol, 48 px, banco.
const fs = require('fs'), path = require('path');
function relojManual(inicioMs) {   // el de tests/dom_claves.js: solo avanza cuando la prueba lo pide
  let t = inicioMs, seq = 1e6;
  const cola = [];
  return { ahora: () => t,
    programar(fn, ms) { const h = ++seq; cola.push({ h, en: t + Math.max(0, +ms || 0), fn }); return h; },
    cancelar(h) {
      const i = cola.findIndex(x => x.h === h);
      if (i >= 0) { cola.splice(i, 1); return true; }
      return false;
    },
    avanzar(ms) {
      const fin = t + ms;
      for (;;) {
        cola.sort((x, y) => x.en - y.en || x.h - y.h);
        if (!cola.length || cola[0].en > fin) break;
        const x = cola.shift(); t = x.en; x.fn();
      }
      t = fin;
    } };
}
module.exports = async function pruebaTresModos(montarAppLimpia, assert) {
  const JERGA = require('./dom_usabilidad.js').JERGA;
  const xor = (s) => {
    let c = 0; for (const ch of s) c ^= ch.charCodeAt(0);
    return c.toString(16).toUpperCase().padStart(2, '0');
  };
  const micro = () => new Promise(r => setTimeout(r, 0));
  const PESTANAS = ['tab-btn-tiempos', 'tab-btn-diag', 'tab-btn-depuracion'];
  const montar = (precarga) => {
    const a = montarAppLimpia(precarga), el = (id) => a.d.getElementById(id);
    a.el = el;
    a.cls = (c) => a.d.body.classList.contains(c);
    a.entra = (c) => a.w._btSubscribeCb(`$${c}*${xor(c)}\n`);
    a.activo = (id) => !!el(id) && el(id).classList.contains('active');
    a.visible = (id) => !!el(id) && el(id).style.display !== 'none' && !el(id).hidden;
    a.pulsa = (id) => { if (el(id)) el(id).click(); };
    a.clave = (v) => {
      a.pulsa('btn-toggle-role');
      if (el('clave-entrada')) { el('clave-entrada').value = v; a.pulsa('btn-clave-entrar'); }
    };
    a.pin = () => ['1', '2', '3', '4'].forEach(k => a.d.querySelector(`.pin-btn[data-key="${k}"]`).click());
    a.txt = (id) => (el(id) ? el(id).textContent.replace(/\s+/g, ' ') : '');
    a.ver = () => { const A = a.w.AsistenteDeg; if (A) A.render(); return A ? A._key : null; };
    // Lo que la persona contesta despues de pulsar: paleteros y PIN.
    a.resolver = () => {
      for (let i = 0; i < 3; i++) {
        if (a.activo('aviso-deg-modal')) { el('chk-aviso-deg').click(); a.pulsa('btn-aviso-deg-confirmar'); }
        if (a.activo('dialogo-modal')) a.pulsa('btn-dialogo-aceptar');
        if (a.activo('pin-modal')) a.pin();
      }
    };
    a.ordenes = () => a.tramas.map(t => t.replace(/^CMD:(PIN:\d+:)?|\r\n$/g, ''));
    a.tramas.length = 0;
    return a;
  };
  const ST = (nodo, serie, modo, rf) => `STATUS,NODE:${nodo},SERIE:${serie},MODO:${modo},ESTADO:ROJO,T:9,RF:${rf},` +
                                         'RTT:0,BAT:12.9,HORA:14:31:00';
  const RTC_OK = 'ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,FECHA:2026-11-10,HORA:14:31:00';
  const seg = (h) => { const p = h.split(':').map(Number); return p[0] * 3600 + p[1] * 60 + p[2]; };
  const difMin = (a0, b0) => (((seg(b0) - seg(a0)) % 86400) + 86400) % 86400 / 60;

  // M1: sin clave. Ni pestanas de tecnico ni asistente; «Practicar sin poste» en la hoja de conexion, a la vista.
  {
    const a = montar();
    assert(PESTANAS.every(id => !a.visible(id)) && !a.cls('admin') && !a.cls('deg') && !!a.el('asistente-deg') &&
           a.el('asistente-deg').hidden,
      'M1 (D-177): Operario sin clave no ve Tiempos, Ajustes, Tramas ni el asistente');
    a.pulsa('btnDevice');
    const bp = a.el('btn-practica');
    assert(a.activo('bt-modal') && !!bp && !!bp.closest('#bt-modal') &&
           !bp.closest('.admin-only, .solo-admin, [hidden]'),
      'M1 (D-177): «Practicar sin poste» esta en la hoja de conexion y la ve el Operario');
  }

  // M2: la clave de Degradado de fabrica (0000) abre SOLO el asistente.
  {
    const a = montar();
    a.clave('0000');
    a.ver();
    assert(a.txt('role-label') === 'Degradado' && a.cls('deg') && !a.cls('admin') &&
           PESTANAS.every(id => !a.visible(id)) && !a.el('asistente-deg').hidden,
      `M2 (D-177): 0000 -> «${a.txt('role-label')}», body.deg, sin pestanas de tecnico y el asistente abierto`);
    a.pulsa('asis-cerrar'); a.ver();
    const volver = a.visible('asistente-volver') && a.el('asistente-deg').hidden;
    a.pulsa('btn-toggle-role'); await micro(); a.ver();
    assert(volver && a.txt('role-label') === 'Operario' && !a.cls('deg') && a.el('asistente-deg').hidden &&
           !a.visible('asistente-volver'),
      'M2: «Ver el semaforo» deja la barra «Volver al asistente»; la pastilla vuelve a Operario y lo oculta todo');
  }

  // M3 (D-57): la de soporte abre el Degradado; su valor no esta en el repo (variable CLAVE_SOPORTE).
  {
    const sop = (typeof process !== 'undefined' && process.env.CLAVE_SOPORTE) || '';
    if (sop) {
      const a = montar(); a.clave(sop);
      assert(a.cls('deg'), 'M3: la de soporte abre el Degradado');
    }
  }

  // M4: la de tecnico (1234) da todo; cambia la de Degradado sin pedir la anterior; no pueden coincidir.
  {
    const a = montar();
    a.clave('1234');
    const tecnico = a.txt('role-label') === 'Técnico' && PESTANAS.every(id => a.visible(id)) &&
                    !!a.el('btn-clave-deg') && !!a.el('btn-clave-deg').closest('#tab-diag.admin-only');
    a.pulsa('btn-clave-deg');
    a.el('clave-entrada').value = '1234'; a.pulsa('btn-clave-entrar');
    const igual = /igual/.test(a.txt('clave-msg'));
    a.el('clave-entrada').value = '5678'; a.pulsa('btn-clave-entrar');
    assert(tecnico && igual && !a.activo('clave-modal'),
      `M4 (D-177): con la de tecnico, «Cambiar la clave de Degradado» la cambia; igual a la de tecnico no: ` +
      `"${a.txt('clave-msg')}"`);
    a.pulsa('btn-toggle-role');
    a.clave('0000');
    const vieja = /incorrecta/i.test(a.txt('clave-msg')) && !a.cls('deg');
    a.pulsa('btn-clave-cancelar'); a.clave('5678');
    assert(vieja && a.cls('deg'), 'M4: la de fabrica ya no entra; la nueva abre el Degradado');
  }

  // M5..M8: el asistente, POSTE 1 y POSTE 2 (cond. 1, 2 y 4), con tramas de verdad.
  let guardado = null;
  {
    const a = montar();
    a.entra(ST('MAESTRO', 'SEM-M-01', 'MENU', 0));
    a.clave('0000');
    const k0 = a.ver();
    a.pulsa('asis-empezar'); const k1 = a.ver();
    const seguirApagado = a.el('asis-seguir1') && a.el('asis-seguir1').disabled;
    a.pulsa('asis-chk1'); a.pulsa('asis-seguir1');
    const k3 = a.ver();
    const v = ['asis-traslado', 'asis-verde', 'asis-despeje'].map(id => (a.el(id) || {}).value).join(',');
    assert(k0 === 'p0' && k1 === 'p1' && seguirApagado && k3 === 'p3' && v === '20,3,30',
      `M5: P0 -> P1 (Seguir apagado sin la casilla) -> P3 con 20 min, 3 min y 30 s: ${k0},${k1},${k3} [${v}]`);
    a.pulsa('asis-poste1'); a.resolver();
    const rtc = a.ordenes().filter(o => /^SET_RTC:/.test(o));
    const degAntes = a.ordenes().filter(o => /DEG_T/.test(o)).length;
    a.entra(RTC_OK);
    const deg = a.ordenes().filter(o => /^SET_MODO:DEG_T:/.test(o));
    const p = deg.length ? deg[0].slice(15).split(',') : [];
    assert(rtc.length === 1 && degAntes === 0 && deg.length === 1 && difMin(p[0], p[1]) === 20 && p[2] === '180' &&
           p[3] === '30',
      `M5: PIN -> SET_RTC y, con su acuse, SET_MODO:DEG_T:ahora,ahora+20min,180,30: ${a.ordenes().join(' | ')}`);
    const k4 = a.ver();
    a.entra('ACK,CMD:SET_MODO:DEG_T,RESULT:OK');
    const k5 = a.ver();
    guardado = a.w.localStorage.getItem('iotvial.testigo.v1');
    assert(k4 === 'p4e' && k5 === 'p5' && a.txt('asis-inicio') === p[1] && /quedan/.test(a.txt('asis-cuerpo')),
      `M6: enviando (${k4}) y, con el acuse, P5 con la hora limite ${a.txt('asis-inicio')} = inicio ${p[1]}`);
    // Cond. 2: a mitad (un poste con testigo y el otro no), en Operario se ve fuera del asistente.
    a.pulsa('btn-toggle-role'); await micro(); a.ver();
    const cartel = a.txt('cartel-testigo-txt');
    assert(a.txt('role-label') === 'Operario' && a.visible('cartel-testigo') && cartel.includes(p[1]) &&
           /paleteros/.test(cartel) && !JERGA.test(cartel),
      `M8 (cond. 2): en Operario, cartel «falta el POSTE 2» con la hora y los paleteros: "${cartel}"`);
  }
  {
    // Cond. 1: la app se cerro entre postes. Al volver, el POSTE 2 pide SU comprobacion en persona antes de enviar.
    const a = montar({ 'iotvial.testigo.v1': guardado || 'null' });
    a.entra(ST('ESCLAVO', 'SEM-E-01', 'SUBORDINADO', 0));
    a.clave('0000');
    const k6 = a.ver(), g = JSON.parse(guardado || '{}');
    a.pulsa('asis-poste2'); a.resolver();
    const sinCasilla = a.ordenes().filter(o => /DEG_T|SET_RTC/.test(o)).length;
    a.pulsa('asis-chk6'); a.pulsa('asis-poste2'); a.resolver();
    a.entra(RTC_OK);
    const deg = a.ordenes().filter(o => /^SET_MODO:DEG_T:/.test(o)), p = deg.length ? deg[0].slice(15).split(',') : [];
    assert(k6 === 'p6' && sinCasilla === 0 && /compru[eé]belo en persona/.test(a.txt('asis-cuerpo')) &&
           deg.length === 1 && p[1] === g.inicio && p[2] === String(g.verde) && p[3] === String(g.despeje),
      `M6 (cond. 1): tras reabrir, P6 no envia sin su casilla (${sinCasilla}); con ella, el MISMO inicio, verde y ` +
      `despeje: ${deg.join(' | ')}`);
    a.entra('ACK,CMD:SET_MODO:DEG_T,RESULT:OK');
    const t = a.w.Testigo.leer();
    assert(!!t && t.esclavo === true && a.ver() === 'p7' && /vuelta completa/.test(a.txt('asis-cuerpo')) &&
           !a.visible('cartel-testigo'),
      'M6: el acuse del POSTE 2 queda en el testigo (esclavo) y P7 pide mirar una vuelta; el cartel se va');
  }
  {
    // Cond. 3: los grupos dicen otra pareja -> no se envia y se dice.
    const grupos = JSON.stringify({ v: 1, grupos: [{ id: 'g1', via: 'Via 1', nombre: 'Km 5', pr: '',
      MAESTRO: { mac: '00:11:22:33:44:55', serie: 'SEM-M-01', fw: null, fecha: 'x' },
      ESCLAVO: { mac: '02:00:00:00:00:99', serie: 'SEM-E-99', fw: null, fecha: 'x' } }] });
    const a = montar({ 'iotvial.testigo.v1': guardado || 'null', 'iot_vial_grupos_v1': grupos });
    a.entra(ST('ESCLAVO', 'SEM-E-01', 'SUBORDINADO', 0));
    a.clave('0000');
    const k = a.ver(), cuerpo = a.txt('asis-cuerpo');
    a.pulsa('asis-poste2'); a.resolver();
    assert(k === 'p6x' && !a.el('asis-poste2') && /no es la pareja/.test(cuerpo) && /SEM-E-99/.test(cuerpo) &&
           !a.ordenes().some(o => /DEG_T/.test(o)),
      `M7 (cond. 3): un POSTE 2 de otra serie que la del grupo: sin boton ni orden, y lo dice (${k}): ` +
      `"${cuerpo.slice(0, 120)}"`);
  }
  {
    // Cond. 4: el rechazo se ensena, con su punta, y el segundo intento no ensena el primero.
    const a = montar();
    a.entra(ST('MAESTRO', 'SEM-M-01', 'MENU', 0));
    a.clave('0000'); a.ver(); a.pulsa('asis-empezar'); a.ver(); a.pulsa('asis-chk1'); a.pulsa('asis-seguir1'); a.ver();
    const enviar = (desc) => {
      a.pulsa('asis-poste1'); a.resolver(); a.entra(RTC_OK); a.entra('ERR,CMD:SET_MODO:DEG_T,DESC:' + desc);
      return [a.ver(), a.txt('asis-cuerpo')];
    };
    const [k1, t1] = enviar('Ahora no coincide');
    a.pulsa('asis-reintentar'); a.ver();
    const [k2, t2] = enviar('Cancele antes la salida programada'), g2 = a.w.Testigo.leer();
    a.pulsa('asis-reintentar'); a.ver();
    const [k3, t3] = enviar('MOTIVO_NUEVO_X');
    assert(k1 === 'p4r' && t1.includes(a.w.Testigo.MOTIVOS['Ahora no coincide']) && k2 === 'p4r' && g2 === null &&
           /cancelela/i.test(t2) && /t[eé]cnico/.test(t2) && !t2.includes('Ahora no coincide') && k3 === 'p4r' &&
           /MOTIVO_NUEVO_X/.test(t3) && !/cancelela/i.test(t3),
      `M7 (cond. 4): rechazos traducidos o crudos, sin el anterior; «Cancele antes» borra el ENVIADO y dice avise ` +
      `al tecnico: [${t1.slice(-60)}] [${t2.slice(-80)}] [${t3.slice(-40)}] guardado ${JSON.stringify(g2)}`);
  }

  // M9: ningun texto del asistente (todas sus pantallas) lleva jerga (SPEC_4 9.9 (a)).
  {
    const a = montar(), A = a.w.AsistenteDeg;
    const malas = (A ? A.CLAVES : ['?']).filter(k => {
      const h = A ? A.html(k).replace(/<[^>]+>/g, ' ') : 'SET_MODO';
      return !h.trim() || JERGA.test(h);
    });
    assert(!!A && A.CLAVES.length >= 18 && malas.length === 0,
      `M9: las pantallas del asistente sin jerga; malas: ${malas.join(',') || '-'}`);
  }

  // M10: terminar el Degradado (salida programada) en los dos postes; con la radio de vuelta no programa nada.
  {
    const a = montar();
    a.entra(ST('MAESTRO', 'SEM-M-01', 'DEGRADADO', 0));
    a.clave('0000'); a.ver(); a.pulsa('asis-ir-salir');
    const k1 = a.ver(), hora = a.txt('asis-hora-s1');
    a.pulsa('asis-fin1'); a.resolver();
    const fin = a.ordenes().filter(o => /^SET_MODO:DEG_FIN:/.test(o));
    const p = fin.length ? fin[0].slice(17).split(',') : [];
    a.entra('ACK,CMD:SET_MODO:DEG_FIN,RESULT:PROGRAMADA');
    const k2 = a.ver();
    a.entra(ST('ESCLAVO', 'SEM-E-01', 'DEGRADADO', 0));
    const k3 = a.ver();
    a.tramas.length = 0; a.pulsa('asis-fin2'); a.resolver();
    const fin2 = a.ordenes().filter(o => /^SET_MODO:DEG_FIN:/.test(o));
    a.entra('ACK,CMD:SET_MODO:DEG_FIN,RESULT:PROGRAMADA');
    assert(k1 === 's1' && fin.length === 1 && difMin(p[0], p[1]) === 20 && hora === p[1].slice(0, 5) && k2 === 's2' &&
           k3 === 's3' && fin2.length === 1 && fin2[0].split(',')[1] === p[1] && a.ver() === 's4',
      `M10: S1 (${hora}) -> DEG_FIN:ahora,ahora+20min -> S2 -> POSTE 2 la misma salida -> S4: ` +
      `${fin.concat(fin2).join(' | ')} ${k1},${k2},${k3}`);
    const b = montar();
    b.entra(ST('MAESTRO', 'SEM-M-01', 'DEGRADADO', 97));
    b.clave('0000'); b.ver(); b.pulsa('asis-ir-salir');
    assert(b.ver() === 's1radio' && !b.el('asis-fin1') && /VOLVER AL MEN[UÚ]/.test(b.txt('asis-cuerpo')),
      `M10: con la radio de vuelta no se programa: dice VOLVER AL MENU en el POSTE 1 (${b.ver()})`);
  }
  // M14 (QA D1): la salida enviada sin respuesta no deja el asistente bloqueado: se puede programar otra vez.
  {
    const a = montar();
    a.entra(ST('MAESTRO', 'SEM-M-01', 'DEGRADADO', 0));
    a.clave('0000'); a.ver(); a.pulsa('asis-ir-salir'); a.ver();
    a.pulsa('asis-fin1'); a.resolver();
    const k1 = a.ver(); a.pulsa('asis-fin-otra'); const k2 = a.ver();
    assert(k1 === 's1e' && k2 === 's1' && a.w.DegFin.leer() === null,
      `M14: salida ENVIADO sin acuse -> «programar otra vez» -> S1, nada guardado: ${k1},${k2}`);
  }
  // M15 (QA D3): testigo ENVIADO y la app en el POSTE 2: dice que vuelva al POSTE 1, sin «Enviando».
  {
    const g = { estado: 'ENVIADO', serie: 'SEM-M-01', inicio: '23:59:00', inicioMs: Date.now() + 3600e3, verde: 180,
                despeje: 30, emitidoMs: Date.now() - 60000 };
    const a = montar({ 'iotvial.testigo.v1': JSON.stringify(g) });
    a.entra(ST('ESCLAVO', 'SEM-E-01', 'SUBORDINADO', 0));
    a.clave('0000'); const k = a.ver(), t = a.txt('asis-cuerpo');
    assert(k === 'p4e' && /Vuelva al POSTE 1/.test(t) && !/Enviando/.test(t) && !a.el('asis-reenviar'),
      `M15: testigo sin acuse visto desde el POSTE 2 -> «Vuelva al POSTE 1»: ${k} "${t.slice(0, 90)}"`);
  }

  // M11: practica SIN clave; el asistente contra el simulador; el puente contesta el SET_RTC (no a los 10 s); nada
  // de lo practicado queda al salir (testigo en memoria, PIN).
  {
    const a = montar();
    const R = relojManual(Date.now() - 2100);   // tras avanzar 2,1 s queda en la hora real: `ahora` cuadra
    if (a.w.Practica) a.w.Practica.reloj = R;
    a.pulsa('btn-bt-disconnect'); a.pulsa('btnDevice'); a.pulsa('btn-practica');
    const entro = a.cls('simulado') && !a.cls('admin');
    const w = a.w, clr = w.clearTimeout.bind(w);
    w.setTimeout = (fn, ms, ...x) => R.programar(() => fn(...x), ms);
    w.clearTimeout = (h) => { if (!R.cancelar(h)) clr(h); };
    const fila = a.d.querySelector('#bt-modal .bt-device-item[data-mac="02:00:00:00:00:05"]');
    if (fila) fila.click();
    const oidas = [], Pp = w.PuertoPractica;
    if (Pp && Pp._cb) { const cb = Pp._cb; Pp._cb = (t) => { oidas.push(String(t).trim()); cb(t); }; }
    R.avanzar(2100);
    a.clave('0000'); a.ver(); a.pulsa('asis-empezar'); a.ver(); a.pulsa('asis-chk1'); a.pulsa('asis-seguir1');
    const k3 = a.ver();
    a.pulsa('asis-poste1'); a.resolver();
    for (let i = 0; i < 4; i++) { R.avanzar(150); a.resolver(); }
    const rtc = oidas.find(t => /CMD:SET_RTC/.test(t)) || '', deg = oidas.find(t => /CMD:SET_MODO:DEG_T/.test(t)) || '';
    const g = w.Testigo.leer();
    const PUENTE_OK = /^\$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,FECHA:\d{4}-\d\d-\d\d,HORA:\d\d:\d\d:\d\d\*/;
    assert(entro && k3 === 'p3' && PUENTE_OK.test(rtc) && /RESULT:OK/.test(deg) && !!g && g.estado === 'ACEPTADO' &&
           a.ver() === 'p5' &&
           a.tramas.length === 0 && w.localStorage.getItem('iotvial.testigo.v1') === null,
      `M11: practica sin clave, Km 120: el puente contesta «${rtc.slice(0, 50)}» y el POSTE 1 «${deg.slice(0, 40)}» ` +
      `en 600 ms (no a los 10 s); el testigo vive en memoria (${g ? g.estado : '-'})`);
    const lit = fs.readFileSync(path.join(__dirname, '..', '..', '..', '01_Firmware', 'ESP32_Expansion', 'src',
                                          'despachador.cpp'), 'utf8');
    assert(lit.includes('"$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,"') &&
           lit.includes('"FECHA:%04d-%02d-%02d,HORA:%02d:%02d:%02d"') &&
           lit.includes('"$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:FORMATO_INVALIDO"'),
      'M11: el literal del puente que imita js/sim_puente.js sigue en despachador.cpp');
    a.pulsa('btn-franja-salir');
    assert(entro && !!g && w.Testigo.leer() === null && w.localStorage.getItem('iotvial.testigo.v1') === null &&
           w.Testigo.ctx.state.pinVerificado === false && !a.cls('simulado'),
      'M11 (D-177): al Salir no queda testigo practicado ni el PIN tecleado en la practica');
  }

  // M12, M13: Averias con selectores (cond. 5: sin consulta tambien se manda).
  const sel = (a, k) => a.el('sel-averia-' + k);
  const elegir = (a, v) => {
    Object.keys(v).forEach(k => { if (sel(a, k)) sel(a, k).value = v[k]; });
    if (a.el('interruptores')) a.el('interruptores').dispatchEvent(new a.w.Event('change'));
  };
  const abrir = (a) => {
    a.clave('1234');
    const av = a.el('averias'); av.open = true; av.dispatchEvent(new a.w.Event('toggle'));
  };
  const nuestras = (a) => a.ordenes().filter(o => /VETO|CAMARA|BARRERA/.test(o));
  {
    const a = montar();
    a.entra(ST('MAESTRO', 'SEM-M-01', 'AUTO', 97) + ',PLUMA:FUERA');
    abrir(a);
    const consulta = nuestras(a).join('|');
    a.entra('ACK,CMD:CONSULTA_VETO,RESULT:C1O_D0?_M1A_M2B');
    const vals = ['C', 'D', 'M1', 'M2', 'B'].map(k => (sel(a, k) || {}).value).join(',');
    assert(consulta === 'CONSULTA_VETO' && vals === 'ON,OFF,ARRIBA,BAJA,OFF' && a.el('btn-averias-aplicar').disabled,
      `M12: preseleccionados con CONSULTA_VETO y la pluma del $STATUS (${vals}); «Aplicar» apagado sin cambios`);
    a.entra('ACK,CMD:CONSULTA_VETO,RESULT:C1O_D1O_M1A_M2B');
    a.tramas.length = 0;
    elegir(a, { C: 'OFF', D: 'OFF', M2: 'ARRIBA', B: 'ON' }); a.pulsa('btn-averias-aplicar');
    const dialogos = a.activo('dialogo-modal') ? 1 : 0;
    a.resolver();
    const primero = nuestras(a).join('|');
    a.tramas.length = 0; a.entra('ACK,CMD:SET_VETO_CAMARA:OFF,RESULT:OK');
    const segundo = nuestras(a).sort().join('|');
    a.tramas.length = 0; a.entra('ERR,CMD:SET_CAMARA_MALA:2:ARRIBA,DESC:NO_QUEDO_PUESTO');
    const ev = a.txt('event-feed');
    assert(dialogos === 1 && primero === 'SET_VETO_CAMARA:OFF' &&
           segundo === 'CONSULTA_VETO|SET_CAMARA_MALA:2:ARRIBA' && !nuestras(a).length && /No se aplicaron/.test(ev) &&
           /barrera en servicio/.test(ev),
      `M13: un dialogo, C y D juntas en UNA orden, la siguiente con el acuse, y un rechazo corta la cola y lo ` +
      `dice: [${primero}] [${segundo}] "${(ev.match(/No se aplicaron[^.]*\./) || [''])[0]}"`);
  }
  {
    const a = montar();
    abrir(a);
    const blanco = ['C', 'D', 'M1', 'M2', 'B'].every(k => sel(a, k) && sel(a, k).value === '') &&
                   a.el('btn-averias-aplicar').disabled;
    a.w.Interruptores.PLAZO_MS = 30;
    a.tramas.length = 0;
    elegir(a, { B: 'OFF', M1: 'BAJA' }); a.pulsa('btn-averias-aplicar'); a.resolver();
    const sale = nuestras(a).join('|');
    await new Promise(r => setTimeout(r, 80));
    const cola = a.txt('averias-cola');
    assert(blanco && sale === 'SET_CAMARA_MALA:1:BAJA' && /No se aplicaron \(el equipo no contest[oó]\)/.test(cola) &&
           /sacar la barrera de servicio/.test(cola),
      `M12 (cond. 5): sin CONSULTA_VETO contestada, en blanco y «Aplicar» manda lo elegido; sin acuse, se dice: ` +
      `[${sale}] "${cola}"`);
  }
};
