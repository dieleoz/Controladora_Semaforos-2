// ===== js/simulador.js =====
// SIMULADOR CON ESTADO DE LA PRACTICA (SPEC_4 9.14 (f); D-120, D-121). Cada poste es un estado (modo, luz, tiempos,
// franjas de noche, tr, Degradado, radio, FW) que contesta cada orden de SPEC_4 3.1 / 3.2 con las tramas que
// emitiria SU firmware: las ramas de procesarComando() de 01_Firmware/{Maestro,Esclavo}/src/bluetooth.cpp y de
// modoNoche_orden() de Maestro/src/modo_noche.cpp, en el mismo orden y con sus literales. Determinista: el tiempo lo
// da un reloj inyectado. No pinta nada: lo usa js/practica.js en el sitio del puerto falso.
// Lo que NO modela (y una respuesta que dependa de ello sale de la rama comun): la radio real entre postes, el reloj
// de respaldo (REINICIAR_RELOJ contesta siempre que el cristal arranca), la flash (nunca FLASH_NO_RELEE ni
// NO_GUARDADO), la transicion del ciclo (CAMBIAR_TURNO nunca EN_TRANSICION_REINTENTE), las camaras (CAM:?) y el
// puente ESP32 (LEER_RTC, SET_RTC y LOG no se contestan: SPEC_4 3.3 es otra punta).
// Verde en practica dice que la app pinta bien, no que el equipo lo haga: no es evidencia (D-121).
const Simulador = (() => {
  // Cada constante, copiada del firmware, con su origen `archivo:simbolo` (tests/dom_simulador.js lo busca alli).
  const BT = 'Maestro/src/bluetooth.cpp:', LC = 'Maestro/include/limites_ciclo.h:';
  const MD = 'Maestro/src/modo_degradado.cpp:';
  const DEF = {
    STATUS_MS: [2000, BT + 'tUltimaTelemetria'], PIN: ['1234', BT + 'CMD:PIN:'],
    AMARILLO_SEG: [3, 'Maestro/include/protocolo.h:AMARILLO_SEG'],
    ROJO_AMARILLO_SEG: [2, 'Maestro/include/protocolo.h:ROJO_AMARILLO_SEG'],
    VERDE_MIN_MIN: [3, LC + 'VERDE_MIN_MIN'], VERDE_MIN_MAX: [15, LC + 'VERDE_MIN_MAX'],
    ROJO_MIN_MIN: [3, LC + 'ROJO_MIN_MIN'], ROJO_MIN_MAX: [15, LC + 'ROJO_MIN_MAX'],
    DESPEJE_SEG_MIN: [10, LC + 'DESPEJE_SEG_MIN'], DESPEJE_SEG_MAX: [90, LC + 'DESPEJE_SEG_MAX'],
    NOCHE_TR_FABRICA_S: [180, 'Maestro/include/modo_noche.h:NOCHE_TR_FABRICA_S'],
    NOCHE_TR_MAX_S: [900, 'Maestro/include/modo_noche.h:NOCHE_TR_MAX_S'],
    NOCHE_FRANJAS: [4, 'Maestro/include/noche_flash.h:NOCHE_FRANJAS'],
    TOLERANCIA_TESTIGO_S: [3, MD + 'TOLERANCIA_TESTIGO_S'], TESTIGO_DESPEJE_MIN: [30, MD + 'TESTIGO_DESPEJE_MIN'],
    TESTIGO_DESPEJE_MAX: [255, MD + 'TESTIGO_DESPEJE_MAX'], TESTIGO_INICIO_MAX_S: [43200, MD + 'TESTIGO_INICIO_MAX_S'],
    ROJO_MINIMO_MS: [4000, 'Esclavo/src/modo_degradado.cpp:ROJO_MINIMO_MS'],
    DEG_AUTO_ACUSE_MS: [10000, 'Maestro/include/deg_auto.h:DEG_AUTO_ACUSE_MS'],
    FW_SIN_SELLO: ['--', 'Maestro/include/version_fw.h:FW_SIN_SELLO']
  };
  const C = {}, ORIGEN = {};
  Object.keys(DEF).forEach(k => { C[k] = DEF[k][0]; ORIGEN[k] = DEF[k][1]; });
  const PREF_PIN = 'CMD:PIN:' + C.PIN + ':';
  // Las que el despachador del Maestro admite sin PIN (bluetooth.cpp, la lista de antes de AUTH_FAILED).
  const SIN_PIN_M = ['SET_MODO:MENU', 'SET_MODO:ALCANCE', 'CONSULTA_DEG_AUTO', 'CONSULTA_DEG_FIN', 'REINICIOS',
                     'VERSION'];
  const DIAS = 'LMXJVSD';

  // Checksum NMEA de enviarTramaConCrc(): XOR de todo lo que va tras el '$'.
  function conCrc(p) {
    let c = 0;
    for (let i = 1; i < p.length; i++) c ^= p.charCodeAt(i) & 0xFF;
    return p + '*' + c.toString(16).toUpperCase().padStart(2, '0');
  }
  const dd = (n) => String(n).padStart(2, '0');

  // El reloj: { ahora(), programar(fn, ms), cancelar(h) }. El real es el del telefono; la prueba inyecta otro.
  function relojReal() {
    return { ahora: () => Date.now(), programar: (fn, ms) => setTimeout(fn, ms), cancelar: (h) => clearTimeout(h) };
  }

  // def = { nodo, serie, direccion, rotulo, fw, sellado, modo, radio, rf, rtt, tiempos:{v,r,d},
  //         noche:{tr, franjas:[{dias,inicio,fin,rojo}], fase, rojo}, enHora, desde }
  function crear(opciones) {
    const reloj = opciones.reloj || relojReal();
    const emitir = opciones.emitir || (() => {});
    const timers = new Set();
    const postes = (opciones.postes || []).map(nuevoPoste);
    postes.forEach(p => { if (p.pareja) p.otro = postes.find(q => q !== p && q.pareja === p.pareja) || null; });

    function nuevoPoste(d) {
      const p = Object.assign({ sellado: true, radio: true, rf: null, rtt: null, enHora: true, pareja: null,
                                degAuto: false, salidaS: 0, ambarLatch: false, rojoFijo: false, testLeds: 0,
                                cicloDesde: 0, saliendoDesde: 0, nocheDesde: 0, relojPendiente: false,
                                rein: 0, causa: 'CORTE', antes: 0 }, d);
      p.tiempos = Object.assign({ v: C.VERDE_MIN_MIN, r: C.ROJO_MIN_MIN, d: C.DESPEJE_SEG_MIN }, d.tiempos || {});
      const n = d.noche || {}, fr = n.franjas || [];
      p.franjas = [];
      for (let i = 0; i < C.NOCHE_FRANJAS; i++) p.franjas.push(Object.assign({ dias: 0, inicio: 0, fin: 0 }, fr[i]));
      Object.assign(p, { tr: n.tr || C.NOCHE_TR_FABRICA_S, nocheFase: n.fase || 'ENTRANDO',
                         nocheDestino: n.destino || 'AUTO', nocheActiva: p.modo === 'NOCHE' ? 0 : -1,
                         nocheOrigen: 'AUTO', degSaliendo: !!d.degSaliendo, ultimoMinuto: -1 });
      const t0 = reloj.ahora() - (d.desdeS || 0) * 1000;
      p.cicloDesde = t0; p.nocheDesde = t0; p.saliendoDesde = t0; p.encendido = t0;
      return p;
    }

    const programar = (fn, ms) => { const h = reloj.programar(() => { timers.delete(h); fn(); }, ms); timers.add(h); };
    function parar() { timers.forEach(h => reloj.cancelar(h)); timers.clear(); }

    // ---- reloj del poste ----
    const fecha = () => new Date(reloj.ahora());
    const segDia = () => { const f = fecha(); return f.getHours() * 3600 + f.getMinutes() * 60 + f.getSeconds(); };
    const horaTxt = (p) => (!p.enHora ? '--:--:--'
      : [fecha().getHours(), fecha().getMinutes(), fecha().getSeconds()].map(dd).join(':'));
    const difCircular = (a, b) => { const d = (a + 86400 - b) % 86400; return d > 43200 ? d - 86400 : d; };

    function evento(p, origen, detalle) {
      emitir(p, conCrc('$EVENT,NODE:' + p.nodo + ',ORIGEN:' + origen + ',DETALLE:' + detalle + ',HORA:' + horaTxt(p)));
    }

    // ---- la luz que da cada modo (semaforo_nombreEstado / coordinador_estadoEsclavo) ----
    function ciclo(p, desde) {
      const t = p.tiempos, s = Math.floor((reloj.ahora() - desde) / 1000);
      const fases = [['ROJO+AMAR', 'ROJO', C.ROJO_AMARILLO_SEG], ['VERDE', 'ROJO', t.v * 60],
                     ['AMARILLO', 'ROJO', C.AMARILLO_SEG], ['ROJO', 'ROJO', t.d], ['ROJO', 'ROJO', C.ROJO_AMARILLO_SEG],
                     ['ROJO', 'VERDE', t.r * 60 + C.AMARILLO_SEG], ['ROJO', 'ROJO', t.d]];
      const total = fases.reduce((a, f) => a + f[2], 0);
      let k = ((s % total) + total) % total;
      for (const f of fases) {
        if (k < f[2]) return { estado: f[0], esc: f[1], t: String(f[2] - k) };
        k -= f[2];
      }
      return { estado: 'ROJO', esc: 'ROJO', t: '--' };
    }
    function luzMaestro(p) {
      avanzar(p);
      const sin = { t: '--' };
      if (p.testLeds > reloj.ahora()) return Object.assign({ estado: 'VERDE', esc: 'ROJO' }, sin);
      if (p.modo === 'DEGRADADO') {
        if (p.rojoFijo || p.degSaliendo) return Object.assign({ estado: 'ROJO', esc: '?' }, sin);
        return Object.assign(ciclo(p, p.cicloDesde), { esc: '?', t: '--' });
      }
      if (p.modo === 'AMBAR') return Object.assign({ estado: 'FALLO COM', esc: 'AMBAR' }, sin);
      if (p.modo === 'NOCHE') return { estado: p.nocheFase === 'DESTELLO' ? 'AMBAR_INT' : 'ROJO', esc: '?', t: '--' };
      if (!p.radio) return Object.assign({ estado: 'FALLO COM', esc: p.modo === 'MENU' ? 'ROJO' : '?' }, sin);
      const parado = p.rojoFijo || ['MENU', 'ALCANCE', 'MANUAL'].indexOf(p.modo) >= 0;
      if (parado) return { estado: 'ROJO', esc: 'ROJO', t: '--' };
      return ciclo(p, p.cicloDesde);
    }
    function luzEsclavo(p) {
      avanzar(p);
      if (p.modo === 'DEGRADADO') return p.degSaliendo || p.rojoFijo ? 'ROJO' : ciclo(p, p.cicloDesde).estado;
      if (p.modo === 'RENDIDO' || p.ambarLatch) return 'FALLO COM';
      const m = p.otro;
      if (!p.radio || !m || !m.radio || m.nodo !== 'MAESTRO') return 'FALLO COM';
      const l = luzMaestro(m);
      const rojo = m.franjas[Math.max(0, m.nocheActiva)].rojo;   // coordinador_entrarNoche(rojoPoste2)
      if (m.modo === 'NOCHE' && m.nocheFase === 'DESTELLO') return rojo ? 'ROJO_INT' : 'AMBAR_INT';
      if (m.modo === 'AMBAR') return 'FALLO COM';
      return l.esc === 'VERDE' ? 'VERDE' : 'ROJO';
    }
    const estadoLuz = (p) => (p.nodo === 'MAESTRO' ? luzMaestro(p).estado : luzEsclavo(p));
    const plumaArriba = (e) => ['VERDE', 'FALLO COM', 'AMARILLO', 'AMBAR_INT', 'ROJO_INT'].indexOf(e) >= 0;

    // ---- lo que pasa solo con el tiempo: modoNoche_loop/vigilar, salida del Degradado ----
    function setModo(p, m) {
      if (p.modo === 'NOCHE' && m !== 'NOCHE') { p.nocheActiva = -1; p.nocheFase = 'ENTRANDO'; }
      p.modo = m; p.rojoFijo = false; p.cicloDesde = reloj.ahora();
    }
    function avanzar(p) {
      const ahora = reloj.ahora();
      if (p.modo === 'NOCHE' && p.nocheFase !== 'DESTELLO' && ahora - p.nocheDesde >= p.tr * 1000) {
        if (p.nocheFase === 'SALIENDO') setModo(p, p.nocheDestino);
        else p.nocheFase = 'DESTELLO';
      }
      if (p.degSaliendo && ahora - p.saliendoDesde >= Math.max(p.tiempos.d * 1000, C.ROJO_MINIMO_MS)) {
        p.degSaliendo = false; p.salidaS = 0;
        setModo(p, p.nodo === 'MAESTRO' ? 'MENU' : 'SUBORDINADO');
      }
      if (p.nodo === 'MAESTRO' && p.enHora) vigilarNoche(p);
    }
    function vigilarNoche(p) {
      const f = fecha(), m = Math.floor(reloj.ahora() / 60000);
      if (m === p.ultimoMinuto) return;
      const primero = p.ultimoMinuto < 0;
      p.ultimoMinuto = m;
      if (primero) return;
      const minDia = f.getHours() * 60 + f.getMinutes(), dia = (f.getDay() + 6) % 7;
      for (let i = 0; i < C.NOCHE_FRANJAS; i++) {
        const fr = p.franjas[i];
        if (fr.dias === 0 || fr.inicio === fr.fin || minDia !== fr.inicio || ((fr.dias >> dia) & 1) === 0) continue;
        if (p.modo === 'AUTO' || p.modo === 'INTELIGENTE') {
          p.nocheOrigen = p.modo; p.nocheActiva = i;
          p.modo = 'NOCHE'; p.nocheFase = 'ENTRANDO'; p.nocheDesde = reloj.ahora();
          evento(p, 'NOCHE', 'ENTRADA');
          return;
        }
        evento(p, 'NOCHE', 'FRANJA_SALTADA');
      }
    }

    // ---- $STATUS de cada punta (el snprintf de bluetooth.cpp, campo por campo) ----
    function status(p) {
      if (p.nodo === 'MAESTRO') {
        const l = luzMaestro(p);
        const rf = p.rf === null ? '--' : p.rf + '%', rtt = p.rf === null ? '--' : p.rtt + 'ms';
        return conCrc('$STATUS,NODE:MAESTRO,SERIE:' + p.serie + ',MODO:' + p.modo + ',ESTADO:' + l.estado + ',T:' +
                      l.t +
                      ',RF:' + rf + ',RTT:' + rtt + ',BAT:--,HORA:' + horaTxt(p) + ',ESC:' + l.esc + ',PLUMA:' +
                      (plumaArriba(l.estado) ? 'ARRIBA' : 'ABAJO') + ',CAM:?');
      }
      const e = luzEsclavo(p);
      return conCrc('$STATUS,NODE:ESCLAVO,SERIE:' + p.serie + ',MODO:' + p.modo + ',ESTADO:' + e +
                    ',T:--,RF:--,RTT:--,BAT:--,HORA:' + horaTxt(p) + ',PLUMA:' + (plumaArriba(e) ? 'ARRIBA' : 'ABAJO') +
                    ',CAM:?');
    }

    // ---- respuestas comunes a las dos puntas ----
    const version = (p) => (p.sellado ? '$ACK,CMD:VERSION,RESULT:OK,NODE:' + p.nodo + ',FW:' + p.fw
                                      : '$ERR,CMD:VERSION,DESC:SIN_SELLAR,NODE:' + p.nodo + ',FW:' + C.FW_SIN_SELLO);
    const reinicios = (p) => '$ACK,CMD:REINICIOS,RESULT:OK,NODE:' + p.nodo + ',REIN:' + p.rein + ',CAUSA:' + p.causa +
                             ',UP:' + Math.floor((reloj.ahora() - p.encendido) / 1000) + ',ANTES:' + p.antes;
    const aptoPropio = (p) => p.degAuto && p.radio && p.modo !== 'DEGRADADO';
    const consultaDegAuto = (p) => '$ACK,CMD:CONSULTA_DEG_AUTO,RESULT:ESTE_' + (p.degAuto ? 'ON' : 'OFF') + '_OTRO_' +
      (p.otro && p.radio && p.otro.degAuto ? 'ON' : 'OFF') + '_APTO_' + (aptoPropio(p) ? 'SI' : 'NO');
    function consultaDegFin(p) {
      if (!p.salidaS) return '$ACK,CMD:CONSULTA_DEG_FIN,RESULT:NINGUNA';
      let falta = (p.salidaS + 86400 - segDia()) % 86400;
      if (falta > 43200) falta = 0;
      const s = p.salidaS;
      return '$ACK,CMD:CONSULTA_DEG_FIN,RESULT:SALE_' + dd(Math.floor(s / 3600)) + dd(Math.floor(s / 60) % 60) +
             dd(s % 60) + '_FALTAN_' + falta + 'S_RESPALDADA';
    }
    const hms = (t) => {   // horaDelDiaValida(): -1 si no es una hora del dia
      const m = /^(\d+):(\d+):(\d+)$/.exec(t);
      return m && +m[1] <= 23 && +m[2] <= 59 && +m[3] <= 59 ? +m[1] * 3600 + +m[2] * 60 + +m[3] : -1;
    };
    // SET_MODO:DEG_T:ahora,inicio,verde,despeje (sscanf de 8 campos con centinela) y modo_degradado_entrarTestigo().
    // El orden y los textos son los de cada punta: el Esclavo mira el inicio antes que el verde y lo dice distinto.
    function degT(p, args, tx) {
      const m = /^(\d+:\d+:\d+),(\d+:\d+:\d+),([+-]?\d+),([+-]?\d+)$/.exec(args);
      const ahora = m ? hms(m[1]) : -1, inicio = m ? hms(m[2]) : -1;
      if (!m || ahora < 0 || inicio < 0) return tx('$ERR,CMD:SET_MODO:DEG_T,DESC:FORMATO_INVALIDO');
      if (p.salidaS) return tx('$ERR,CMD:SET_MODO:DEG_T,DESC:Cancele antes la salida programada');
      const v = +m[3], d = +m[4], reloj0 = segDia();
      let motivo = null;
      const esM = p.nodo === 'MAESTRO';
      const vencido = (inicio + 86400 - reloj0) % 86400 > C.TESTIGO_INICIO_MAX_S;
      const txtVencido = esM ? 'Inicio ya vencido' : 'Inicio ya vencido: repita el testigo en el Maestro';
      if (!p.enHora) motivo = 'Falta: reloj sin poner en hora';
      else if (Math.abs(difCircular(ahora, reloj0)) > C.TOLERANCIA_TESTIGO_S) motivo = 'Ahora no coincide';
      else if (!esM && vencido) motivo = txtVencido;
      else if (!(v >= C.VERDE_MIN_MIN * 60 && v <= C.VERDE_MIN_MAX * 60 && v % 60 === 0)) {
        motivo = 'Verde fuera de rango (3-15 min)';
      }
      else if (d < C.TESTIGO_DESPEJE_MIN || d > C.TESTIGO_DESPEJE_MAX) motivo = 'Despeje fuera de rango (30-255)';
      else if (vencido) motivo = txtVencido;
      else if ((esM && p.modo === 'AMBAR') || p.ambarLatch) motivo = 'Ambar de emergencia puesto';
      if (motivo) {
        tx('$ERR,CMD:SET_MODO:DEG_T,DESC:' + motivo);
        return esM ? undefined : evento(p, 'APP_BLUETOOTH', 'SET_MODO_DEG_T_RECHAZADO');
      }
      const renueva = p.modo === 'DEGRADADO' && !p.degSaliendo && p.tiempos.v * 60 === v && p.tiempos.d === d;
      p.tiempos = Object.assign({}, p.tiempos, { v: v / 60, d });
      if (!renueva) { setModo(p, 'DEGRADADO'); p.degSaliendo = false; }
      tx('$ACK,CMD:SET_MODO:DEG_T,RESULT:' + (renueva ? 'RENOVADO' : 'OK'));
      evento(p, 'APP_BLUETOOTH', renueva ? 'SET_MODO_DEG_T_RENOVADO' : 'SET_MODO_DEG_T');
    }
    // SET_MODO:DEG_FIN:ahora,salida | CANCELAR (modo_degradado_programarSalida / cancelarSalida y su texto).
    function degFin(p, args, tx) {
      const cancelar = args === 'CANCELAR';
      const m = /^(\d+:\d+:\d+),(\d+:\d+:\d+)$/.exec(args);
      const ahora = m ? hms(m[1]) : -1, salida = m ? hms(m[2]) : -1;
      if (!cancelar && (ahora < 0 || salida < 0)) return tx('$ERR,CMD:SET_MODO:DEG_FIN,DESC:FORMATO_INVALIDO');
      let r;
      if (p.modo !== 'DEGRADADO') r = 'No esta en Degradado';
      else if (p.degSaliendo) r = 'Ya esta saliendo';
      else if (cancelar) {
        r = !p.salidaS ? 'No hay salida programada'
          : estadoLuz(p) !== 'ROJO' ? 'En verde: repita en rojo' : 'CANCELADA';
      }
      else if (!p.enHora) r = 'Falta: reloj sin poner en hora';
      else if (Math.abs(difCircular(ahora, segDia())) > C.TOLERANCIA_TESTIGO_S) r = 'Ahora no coincide';
      else if ((salida + 86400 - segDia()) % 86400 > 43200) r = 'Salida ya vencida';
      else if (estadoLuz(p) !== 'ROJO') r = 'En verde: repita en rojo';
      else r = p.salidaS ? 'REPROGRAMADA' : 'PROGRAMADA';
      const ok = ['PROGRAMADA', 'REPROGRAMADA', 'CANCELADA'].indexOf(r) >= 0;
      if (ok) p.salidaS = cancelar ? 0 : salida;
      tx((ok ? '$ACK,CMD:SET_MODO:DEG_FIN,RESULT:' : '$ERR,CMD:SET_MODO:DEG_FIN,DESC:') + r);
      const det = cancelar ? 'SALIDA_CANCELADA' : 'SALIDA_PROGRAMADA_' + m[2].split(':').map(x => dd(+x)).join(':');
      if (ok) evento(p, 'DEGRADADO', det);
    }
    // SET_DEG_AUTO:0|1: rechazo en el acto; el $ACK, diferido, cuando el otro poste lo refleja (degAuto_veredicto()).
    function setDegAuto(p, args, tx) {
      if (!/^[01]$/.test(args)) return tx('$ERR,CMD:SET_DEG_AUTO,DESC:FORMATO_INVALIDO');
      if (!p.radio || !p.otro) return tx('$ERR,CMD:SET_DEG_AUTO,DESC:SIN_ENLACE_CON_EL_OTRO_POSTE');
      if (p.modo === 'DEGRADADO') return tx('$ERR,CMD:SET_DEG_AUTO,DESC:EN_DEGRADADO_SALGA_PRIMERO');
      p.degAuto = args === '1';
      evento(p, 'APP_BLUETOOTH', p.degAuto ? 'SET_DEG_AUTO_1' : 'SET_DEG_AUTO_0');
      programar(() => emitir(p, conCrc(!p.degAuto ? '$ACK,CMD:SET_DEG_AUTO,RESULT:OFF'
        : p.otro.degAuto ? '$ACK,CMD:SET_DEG_AUTO,RESULT:ON_EFECTIVO'
                         : '$ACK,CMD:SET_DEG_AUTO,RESULT:ON_FALTA_EL_OTRO_POSTE')),
        C.STATUS_MS);
    }

    // ---- Maestro: modoNoche_orden() y procesarComando(), en ese orden ----
    function contestarNoche(p, tx, cmd, err) {
      tx(err ? '$ERR,CMD:' + cmd + ',DESC:' + err : '$ACK,CMD:' + cmd + ',RESULT:OK');
      if (!err) evento(p, 'APP_BLUETOOTH', cmd);
    }
    function fijarFranja(p, a) {
      let f = { dias: 0, inicio: 0, fin: 0, rojo: false }, n;
      const off = /^([+-]?\d+),([A-Z]{1,3})$/.exec(a);
      const on = /^([+-]?\d+),([LMXJVSD-]{7}),(\d{4}),(\d{4}),([AR])$/.exec(a);
      if (off && off[2] === 'OFF') n = +off[1];
      else if (on) {
        n = +on[1];
        for (let b = 0; b < 7; b++) {
          if (on[2][b] === DIAS[b]) f.dias |= (1 << b);
          else if (on[2][b] !== '-') return 'FORMATO_INVALIDO';
        }
        const hm = (t) => { const h = +t.slice(0, 2), mi = +t.slice(2); return h > 23 || mi > 59 ? -1 : h * 60 + mi; };
        f.inicio = hm(on[3]); f.fin = hm(on[4]);
        if (f.inicio < 0 || f.fin < 0) return 'RANGO';
        f.rojo = on[5] === 'R';
      } else return 'FORMATO_INVALIDO';
      if (n < 1 || n > C.NOCHE_FRANJAS) return 'RANGO';
      if (p.modo === 'NOCHE' && p.nocheActiva === n - 1) return 'EN_NOCHE_SALGA_PRIMERO';
      p.franjas[n - 1] = f;
      return null;
    }
    function fijarTr(p, a) {
      if (!/^[+-]?\d+$/.test(a)) return 'FORMATO_INVALIDO';
      if (+a < C.DESPEJE_SEG_MIN || +a > C.NOCHE_TR_MAX_S) return 'RANGO';
      p.tr = +a;
      return null;
    }
    function consultaNoche(p) {
      let s = '$ACK,CMD:NOCHE,RESULT:OK,TR:' + p.tr;
      const hhmm = (m) => dd(Math.floor(m / 60)) + dd(m % 60);
      p.franjas.forEach((f, i) => {
        let d = '';
        for (let b = 0; b < 7; b++) d += ((f.dias >> b) & 1) ? DIAS[b] : '-';
        const apagada = f.dias === 0 || f.inicio === f.fin;
        s += ',F' + (i + 1) + ':' + d + '-' + hhmm(f.inicio) + '-' + hhmm(f.fin) + '-' +
             (apagada ? 'OFF' : f.rojo ? 'R' : 'A');
      });
      return s;
    }
    function ordenNoche(p, cmd, tx) {
      const conPin = cmd.indexOf(PREF_PIN) === 0;
      if (!conPin && cmd.indexOf('CMD:') !== 0) return false;
      const a = cmd.slice(conPin ? PREF_PIN.length : 4);
      if (a === 'CONSULTA_NOCHE') { tx(consultaNoche(p)); return true; }
      if (conPin && a.indexOf('SET_NOCHE:') === 0) {
        contestarNoche(p, tx, 'SET_NOCHE', fijarFranja(p, a.slice(10)));
        return true;
      }
      if (conPin && a.indexOf('SET_NOCHE_TR:') === 0) {
        contestarNoche(p, tx, 'SET_NOCHE_TR', fijarTr(p, a.slice(13)));
        return true;
      }
      if (p.modo !== 'NOCHE') return false;
      if (conPin && (a.indexOf('SET_MODO:DEG_T:') === 0 || a === 'SET_MODO:AMBAR')) {
        tx('$ERR,CMD:' + a.slice(0, 14) + ',DESC:EN_NOCHE_SALGA_PRIMERO');
        return true;
      }
      if (a === 'SET_MODO:MENU' || a === 'SET_MODO:ALCANCE') { evento(p, 'NOCHE', 'SALIDA_APP'); return false; }
      if (a === 'FORZAR_ROJO') { evento(p, 'NOCHE', 'SALIDA_APP'); setModo(p, 'MANUAL'); return false; }
      const destino = { 'SET_MODO:AUTO': 'AUTO', 'SET_MODO:INTELIGENTE': 'INTELIGENTE',
                        'SET_MODO:MANUAL': 'MANUAL' }[a];
      if (!destino || !conPin) return false;
      if (p.nocheFase === 'SALIENDO') { tx('$ERR,CMD:' + a + ',DESC:SALIDA_EN_CURSO'); return true; }
      p.nocheDestino = destino; p.nocheFase = 'SALIENDO'; p.nocheDesde = reloj.ahora();
      evento(p, 'NOCHE', 'SALIDA_APP');
      tx('$ACK,CMD:' + a + ',RESULT:SALIENDO_TODO_ROJO');
      return true;
    }
    function forzarRojoMaestro(p, tx) {
      if (p.modo !== 'DEGRADADO') { p.rojoFijo = true; return tx('$ACK,CMD:FORZAR_ROJO,RESULT:OK'); }
      if (p.rojoFijo) return tx('$ACK,CMD:FORZAR_ROJO,RESULT:YA_EN_ROJO_FIJO');
      if (p.degSaliendo) return tx('$ACK,CMD:FORZAR_ROJO,RESULT:SALIDA_YA_EN_CURSO');
      p.rojoFijo = true; p.salidaS = 0;
      tx('$ACK,CMD:FORZAR_ROJO,RESULT:ROJO_FIJO_EN_ESTE_POSTE');
    }
    function maestro(p, cmd, tx) {
      if (cmd === '$LATIDO') return;
      if (ordenNoche(p, cmd, tx)) return;
      if (cmd === 'CMD:FORZAR_ROJO') {
        forzarRojoMaestro(p, tx);
        return evento(p, 'APP_BLUETOOTH', 'FORZAR_ROJO_SIN_PIN');
      }
      if (cmd.indexOf('CMD:HORA_ESP32:') === 0) return;   // la origina el puente y no se contesta
      let a;
      if (cmd.indexOf(PREF_PIN) === 0) a = cmd.slice(PREF_PIN.length);
      else if (cmd.indexOf('CMD:') === 0 && SIN_PIN_M.indexOf(cmd.slice(4)) >= 0) a = cmd.slice(4);
      else return tx('$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO');
      const ev = (d) => evento(p, 'APP_BLUETOOTH', d);
      const deg = p.modo === 'DEGRADADO';
      if (a === 'SET_MODO:AUTO') { setModo(p, 'AUTO'); tx('$ACK,CMD:SET_MODO:AUTO,RESULT:OK'); ev('SET_MODO_AUTO'); }
      else if (a === 'SET_MODO:MANUAL') {
        setModo(p, 'MANUAL'); tx('$ACK,CMD:SET_MODO:MANUAL,RESULT:OK'); ev('SET_MODO_MANUAL');
      }
      else if (a === 'SET_MODO:AMBAR') {
        const ya = p.modo === 'AMBAR';
        setModo(p, 'AMBAR');
        tx(ya ? '$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO' : '$ACK,CMD:SET_MODO:AMBAR,RESULT:OK'); ev('SET_MODO_AMBAR');
      } else if (a === 'SET_MODO:MENU') {
        if (deg && !p.degSaliendo) {
          p.degSaliendo = true; p.saliendoDesde = reloj.ahora();
          tx('$ACK,CMD:SET_MODO:MENU,RESULT:SALIENDO_TODO_ROJO'); ev('SALIDA_DEGRADADO_PEDIDA');
        } else if (deg) tx('$ERR,CMD:SET_MODO:MENU,DESC:YA_VUELVE_AL_MENU');
        else { setModo(p, 'MENU'); tx('$ACK,CMD:SET_MODO:MENU,RESULT:OK'); ev('SET_MODO_MENU'); }
      } else if (a === 'SET_MODO:ALCANCE' || a === 'SET_MODO:INTELIGENTE') {
        const m = a.slice(9), c = '$ACK,CMD:' + a + ',RESULT:';
        if (deg) tx('$ERR,CMD:' + a + ',DESC:EN_MARCHA_PARE_EL_MODO');
        else if (p.modo === m) tx(c + 'SIN_CAMBIO');
        else { setModo(p, m); tx(c + 'OK'); ev('SET_MODO_' + m); }
      } else if (a.indexOf('SET_MODO:DEG_T:') === 0) degT(p, a.slice(15), tx);
      else if (a.indexOf('SET_MODO:DEG_FIN:') === 0) degFin(p, a.slice(17), tx);
      else if (a === 'CONSULTA_DEG_FIN') tx(consultaDegFin(p));
      else if (a.indexOf('SET_DEG_AUTO:') === 0) setDegAuto(p, a.slice(13), tx);
      else if (a === 'CONSULTA_DEG_AUTO') tx(consultaDegAuto(p));
      else if (a === 'FORZAR_ROJO') { forzarRojoMaestro(p, tx); ev('FORZAR_ROJO_TOTAL'); }
      else if (a === 'MANUAL:CAMBIAR_TURNO') {
        if (['AUTO', 'INTELIGENTE', 'MANUAL'].indexOf(p.modo) < 0) {
          tx('$ERR,CMD:CAMBIAR_TURNO,DESC:MODO_SIN_CICLO_SALGA_PRIMERO');
        } else {
          p.cicloDesde -= 1000 * Math.max(0, +ciclo(p, p.cicloDesde).t || 0);   // la fase en curso se da por cumplida
          tx('$ACK,CMD:CAMBIAR_TURNO,RESULT:OK'); ev('CAMBIAR_TURNO_MANUAL');
        }
      } else if (a === 'TEST_LEDS') {
        const e = luzMaestro(p).estado, fuera = p.modo === 'MENU' || p.modo === 'ALCANCE';
        if (fuera && e === 'ROJO' && p.radio) {
          p.testLeds = reloj.ahora() + 6000; tx('$ACK,CMD:TEST_LEDS,RESULT:STARTING_6S'); ev('TEST_LEDS_INICIADO');
        } else if (e === 'FALLO COM') tx('$ERR,CMD:TEST_LEDS,DESC:SIN_ENLACE_AMBAR_NO_SE_PRUEBA');
        else if (fuera) tx('$ERR,CMD:TEST_LEDS,DESC:ESPERANDO_ROJO_DEL_ESCLAVO');
        else tx('$ERR,CMD:TEST_LEDS,DESC:EN_SERVICIO_PASE_A_MENU');
      } else if (a.indexOf('SET_TIEMPOS:') === 0) {
        const m = /^([+-]?\d+),([+-]?\d+),([+-]?\d+)$/.exec(a.slice(12));
        if (!m) tx('$ERR,CMD:SET_TIEMPOS,DESC:FORMATO_INVALIDO');
        else if (p.modo === 'AUTO') tx('$ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO');
        else if (+m[1] < C.VERDE_MIN_MIN || +m[1] > C.VERDE_MIN_MAX || +m[2] < C.ROJO_MIN_MIN ||
                 +m[2] > C.ROJO_MIN_MAX ||
                 +m[3] < C.DESPEJE_SEG_MIN || +m[3] > C.DESPEJE_SEG_MAX) tx('$ERR,CMD:SET_TIEMPOS,DESC:RANGO');
        else {
          p.tiempos = { v: +m[1], r: +m[2], d: +m[3] }; tx('$ACK,CMD:SET_TIEMPOS,RESULT:OK'); ev('TIEMPOS_CAMBIADOS');
        }
      } else if (a === 'REINICIAR_RELOJ') {
        if (p.relojPendiente) return tx('$ERR,CMD:REINICIAR_RELOJ,DESC:REPITA_EN_UNOS_SEGUNDOS');
        p.relojPendiente = true; ev('RELOJ_REINICIADO_VERIFICANDO');
        programar(() => {
          p.relojPendiente = false;
          emitir(p, conCrc('$ACK,CMD:REINICIAR_RELOJ,RESULT:CRISTAL_OK_PONGA_LA_HORA'));
        }, C.STATUS_MS);
      } else if (a === 'DEMANDA') {
        if (p.modo !== 'INTELIGENTE') tx('$ERR,CMD:DEMANDA,DESC:SOLO_EN_MODO_INTELIGENTE');
        else { tx('$ACK,CMD:DEMANDA,RESULT:REGISTRADA'); ev('DEMANDA_LOCAL'); }
      } else if (a === 'REINICIOS') tx(reinicios(p));
      else if (a === 'VERSION') tx(version(p));
      else tx('$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO');
    }

    // ---- Esclavo: procesarComando() de Esclavo/src/bluetooth.cpp ----
    function ambarEmergencia(p, tx, conPin) {
      const det = conPin ? 'AMBAR_EMERGENCIA_LOCAL' : 'AMBAR_EMERGENCIA_SIN_PIN';
      if (p.modo !== 'DEGRADADO') {
        const ya = luzEsclavo(p) === 'FALLO COM', sinRadio = !p.radio || !p.otro;
        p.ambarLatch = true;
        // El aviso al Poste 1 no consta en el acto (avisoAmbarConfirmado): sin radio, _SIN_RADIO; con ella, el liso.
        tx(ya ? (sinRadio ? '$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO_SIN_RADIO'
                          : '$ACK,CMD:AMBAR_EMERGENCIA,RESULT:YA_EN_AMBAR_LATCH_PUESTO')
              : (sinRadio ? '$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK_SIN_RADIO' : '$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK'));
      } else if (!p.degSaliendo) {
        p.degSaliendo = true; p.saliendoDesde = reloj.ahora(); p.ambarLatch = true;
        tx('$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO');
      } else {
        tx('$ERR,CMD:AMBAR_EMERGENCIA,DESC:SALIDA_A_ROJO_EN_CURSO_REPITA');
        return evento(p, 'APP_BLUETOOTH', 'AMBAR_EMERGENCIA_RECHAZADO');
      }
      evento(p, 'APP_BLUETOOTH', det);
    }
    function esclavo(p, cmd, tx) {
      if (cmd === '$LATIDO') return;
      if (cmd === 'CMD:AMBAR_EMERGENCIA') return ambarEmergencia(p, tx);
      if (cmd === 'CMD:FORZAR_ROJO') return tx('$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA');
      if (cmd.indexOf('CMD:HORA_ESP32:') === 0) return;
      if (cmd === 'CMD:VERSION') return tx(version(p));
      if (cmd === 'CMD:REINICIOS' || cmd === PREF_PIN + 'REINICIOS') return tx(reinicios(p));
      if (cmd === 'CMD:CONSULTA_DEG_FIN' || cmd === PREF_PIN + 'CONSULTA_DEG_FIN') return tx(consultaDegFin(p));
      if (cmd === 'CMD:CONSULTA_DEG_AUTO' || cmd === PREF_PIN + 'CONSULTA_DEG_AUTO') return tx(consultaDegAuto(p));
      if (cmd.indexOf(PREF_PIN) !== 0) return tx('$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO');
      const a = cmd.slice(PREF_PIN.length);
      if (a === 'AMBAR_EMERGENCIA') ambarEmergencia(p, tx, true);
      else if (a === 'CANCELAR_AMBAR') {
        if (!p.ambarLatch) {
          tx(luzEsclavo(p) === 'FALLO COM' ? '$ACK,CMD:CANCELAR_AMBAR,RESULT:REENVIADO_AL_MAESTRO'
                                           : '$ERR,CMD:CANCELAR_AMBAR,DESC:NO_HAY_AMBAR_VIGENTE');
        } else { p.ambarLatch = false; tx('$ACK,CMD:CANCELAR_AMBAR,RESULT:RETIRADO'); }
      } else if (a === 'FORZAR_ROJO') tx('$ERR,CMD:FORZAR_ROJO,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA');
      else if (a.indexOf('SET_MODO:DEG_T:') === 0) degT(p, a.slice(15), tx);
      else if (a.indexOf('SET_MODO:DEG_FIN:') === 0) degFin(p, a.slice(17), tx);
      else if (a.indexOf('SET_DEG_AUTO:') === 0) setDegAuto(p, a.slice(13), tx);
      else if (a === 'TEST_LEDS') tx('$ERR,CMD:TEST_LEDS,DESC:NO_EN_SERVICIO_USE_EL_MAESTRO');
      else if (a === 'VERSION') tx(version(p));
      else tx('$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO_EN_ESCLAVO');
    }

    // La linea tal como la escribe la app (con su fin de linea): devuelve las respuestas inmediatas, con checksum.
    function recibir(p, linea) {
      const out = [];
      const cmd = String(linea || '').replace(/[\r\n]+$/, '');
      (p.nodo === 'MAESTRO' ? maestro : esclavo)(p, cmd, (t) => out.push(conCrc(t)));
      return out;
    }

    return { postes, recibir, status, parar, reloj, avanzar: (p) => avanzar(p),
             poste: (dir) => postes.find(p => p.direccion === dir) || null };
  }

  return { C, ORIGEN, crear, conCrc, relojReal };
})();

if (typeof module !== 'undefined' && module.exports) module.exports = Simulador;
