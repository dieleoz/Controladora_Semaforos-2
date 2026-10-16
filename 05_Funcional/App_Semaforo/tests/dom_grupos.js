// ===== tests/dom_grupos.js =====
// SPEC_4 9.14 (a)..(e): grupos por via y matricula por MAC (D-117, D-118, D-119, D-123). No corre sola: la llama
// test_dom_execution.js con SU montarAppLimpia y SU assert. Pulsa los botones reales del modal de grupos y de la
// lista de equipos; los datos del poste entran por $STATUS y por el $ACK de VERSION, como del equipo.
// Lo que no mide: que Android entregue la MAC en `address` (telefono), ni la radio entre los dos postes.
const fs = require('fs');
const path = require('path');

module.exports = async function pruebaGrupos(montarAppLimpia, assert) {
  const xor = (p) => { let c = 0; for (const ch of p) c ^= ch.charCodeAt(0); return c.toString(16).toUpperCase().padStart(2, '0'); };
  const MAC1 = '00:11:22:33:44:55', MAC2 = 'AA:BB:CC:00:00:02', MAC3 = 'AA:BB:CC:00:00:03', MAC4 = 'AA:BB:CC:00:00:04';
  const montar = (precarga) => {
    const a = montarAppLimpia(precarga);   // enlaza MAC1 y le llega un $STATUS de MAESTRO con SERIE:SEM-M-01
    const el = (id) => a.d.getElementById(id);
    a.el = el;
    a.trama = (p) => a.w._btSubscribeCb(`$${p}*${xor(p)}\n`);
    a.status = (node, serie) =>
      a.trama(`STATUS,NODE:${node},SERIE:${serie},MODO:AUTO,ESTADO:V1_R2,T:30,RF:97,RTT:70,BAT:12.9,HORA:14:31:00`);
    a.version = (node, fw) => a.trama(`ACK,CMD:VERSION,RESULT:OK,NODE:${node},FW:${fw}`);
    a.clave = () => { el('btn-toggle-role').click(); el('clave-entrada').value = '1234'; el('btn-clave-entrar').click(); };
    a.abierto = () => el('dialogo-modal').classList.contains('active');
    a.contestar = (cola) => {
      while (a.abierto()) {
        const v = cola.length ? cola.shift() : null;
        if (v === null) { el('btn-dialogo-cancelar').click(); break; }
        if (v !== true) el('dialogo-entrada').value = v;
        el('btn-dialogo-aceptar').click();
      }
    };
    a.enlazar = (disp, node, serie) => {
      a.w.bluetoothSerial.list = (ok) => ok([disp]);
      el('btnDevice').click();
      const f = Array.from(a.d.querySelectorAll('.bt-device-item'))
        .find(x => x.getAttribute('data-mac') === (disp.address || disp.id));
      if (f) f.click();
      if (node) a.status(node, serie);
      return !!f;
    };
    a.fichas = () => { el('btn-select-site').click(); return Array.from(a.d.querySelectorAll('#dynamic-site-list .site-card')); };
    a.boton = (n, sel) => { const f = a.ficha(n); return f ? f.querySelector(sel) : null; };
    a.ficha = (n) => a.fichas().find(c => c.querySelector('.site-card-title').textContent.includes(n)) || null;
    a.crear = (via, nombre, pr) => { a.fichas(); el('btn-open-add-site').click(); a.contestar([via, nombre, pr]); };
    a.sitio = () => el('current-site-name').textContent;
    a.guardado = () => JSON.parse(a.w.localStorage.getItem('iot_vial_grupos_v1') || '{"grupos":[]}').grupos;
    a.de = (n) => a.guardado().find(g => g.nombre === n) || {};
    a.eventos = () => Array.from(a.d.querySelectorAll('#event-feed .event-item')).map(e => e.textContent).join(' | ');
    a.colorDe = (re) => {
      const ev = Array.from(a.d.querySelectorAll('#event-feed .event-item')).find(e => re.test(e.textContent));
      return ev ? ev.style.borderLeft : '';
    };
    a.tramas.length = 0;
    return a;
  };

  // 0. Grupos no toca localStorage por su cuenta (almacen inyectable, SPEC_4 9.14 Ficheros e interfaces).
  const ruta = path.join(__dirname, '..', 'js', 'grupos.js');
  const fuente = fs.existsSync(ruta) ? fs.readFileSync(ruta, 'utf8') : '';
  assert(/const Grupos\b/.test(fuente) && !/localStorage/.test(fuente.replace(/\/\/.*$/gm, '')),
    'Grupos: js/grupos.js no nombra localStorage fuera de comentarios: el almacen se le inyecta');

  // 1. Sin matricular: «SIN GRUPO». Sin la clave no se ofrece Matricular, y crear un grupo no envia nada.
  {
    const a = montar();
    assert(/SIN GRUPO/.test(a.sitio()), `Grupos: un poste sin matricular da state.site «SIN GRUPO»: "${a.sitio()}"`);
    a.crear('Via 1', 'Km 20', 'PR 20+000');
    const f = a.ficha('Km 20');
    assert(!!f && a.de('Km 20').via === 'Via 1' && a.de('Km 20').pr === 'PR 20+000',
      `Grupos: «Grupo nuevo» pide via, grupo y PR y lo guarda (${JSON.stringify(a.de('Km 20'))})`);
    assert(!!f && !f.querySelector('.btn-matricular') && a.tramas.length === 0,
      `Grupos: sin la clave de administracion no se ofrece Matricular y no sale trama (${a.tramas.join(' | ')})`);
    assert(!!f && /la app no la ve/i.test(f.textContent),
      `Grupos: la ficha del grupo dice que la radio entre postes la app no la ve: "${f ? f.textContent.slice(0, 160) : ''}"`);
  }

  // 2. Con la clave: matricular sin teclear la serie; no envia ninguna trama; state.site pasa a <via> · <grupo>.
  {
    const a = montar();
    a.version('MAESTRO', 'abc1234');
    a.crear('Via 1', 'Km 20', '');
    a.clave();
    a.tramas.length = 0;
    const f = a.ficha('Km 20');
    const b = f && f.querySelector('.btn-matricular');
    if (b) b.click();
    const h = a.de('Km 20').MAESTRO || {};
    assert(!!b && !a.abierto() && h.mac === MAC1 && h.serie === 'SEM-M-01' && h.fw === 'abc1234' && !!h.fecha,
      `Grupos: matricular toma MAC, serie, punta y FW del equipo sin pedir ningun texto (${JSON.stringify(h)})`);
    assert(a.tramas.length === 0, `Grupos: matricular no envia ninguna trama al equipo (${a.tramas.join(' | ')})`);
    assert(/Via 1 · Km 20/.test(a.sitio()), `Grupos: state.site es «<via> · <grupo>» tras matricular: "${a.sitio()}"`);

    // 3. Hueco ocupado no se pisa: otro MAESTRO no ve Matricular en Km 20 y el hueco sigue con MAC1.
    a.enlazar({ name: 'SEM-M-02-M', id: MAC2, address: MAC2 }, 'MAESTRO', 'SEM-M-02');
    const f2 = a.ficha('Km 20');
    assert(!!f2 && !f2.querySelector('.btn-matricular') && (a.de('Km 20').MAESTRO || {}).mac === MAC1,
      'Grupos: un hueco ocupado por otra MAC no se ofrece ni se pisa');

    // 4. Una MAC en un solo grupo; mover pide confirmar; Cancelar no mueve.
    a.enlazar({ name: 'SEM-M-01-M', id: MAC1, address: MAC1 }, 'MAESTRO', 'SEM-M-01');
    a.crear('Via 1', 'Km 80', '');
    const f3 = a.ficha('Km 80');
    const m = f3 && f3.querySelector('.btn-matricular');
    if (m) m.click();
    const pregunta = a.abierto() ? a.el('dialogo-modal').textContent : '';
    assert(!!m && /mover/i.test(pregunta) && /Km 20/.test(pregunta),
      `Grupos: matricular una MAC que ya esta en otro grupo pide confirmar: "${pregunta.replace(/\s+/g, ' ').slice(0, 120)}"`);
    a.contestar([null]);
    assert((a.de('Km 20').MAESTRO || {}).mac === MAC1 && !a.de('Km 80').MAESTRO,
      'Grupos: Cancelar no mueve el poste');
    const m2 = a.boton('Km 80', '.btn-matricular');
    if (m2) m2.click();
    a.contestar([true]);
    const veces = a.guardado().reduce((n, g) => n + ['MAESTRO', 'ESCLAVO'].filter(p => g[p] && g[p].mac === MAC1).length, 0);
    assert(veces === 1 && (a.de('Km 80').MAESTRO || {}).mac === MAC1 && !a.de('Km 20').MAESTRO,
      `Grupos: una MAC esta en un grupo como mucho; mover la saca del anterior (${veces} huecos con MAC1)`);

    // 5. Quitar del grupo libera el hueco.
    const q = a.boton('Km 80', '.btn-quitar-hueco');
    if (q) q.click();
    assert(!!q && !a.de('Km 80').MAESTRO && /SIN GRUPO/.test(a.sitio()),
      `Grupos: «Quitar del grupo» deja el hueco libre y el poste «SIN GRUPO»: "${a.sitio()}"`);
  }

  // 6. Los cuatro avisos de (c), solo con lo que dice el equipo.
  {
    const a = montar();
    a.version('MAESTRO', 'abc1234');
    a.crear('Via 2', 'PR 45', '');
    a.clave();
    const fm = a.boton('PR 45', '.btn-matricular');
    if (fm) fm.click();
    a.enlazar({ name: 'SEM-E-01-E', id: MAC2, address: MAC2 }, 'ESCLAVO', 'SEM-E-01');
    a.version('ESCLAVO', 'def5678');
    const fe = a.boton('PR 45', '.btn-matricular');
    if (fe) fe.click();
    assert((a.de('PR 45').ESCLAVO || {}).mac === MAC2,
      `Grupos: el Esclavo entra en el hueco ESCLAVO del mismo grupo (${JSON.stringify(a.de('PR 45').ESCLAVO)})`);
    assert(/abc1234/.test(a.eventos()) && /def5678/.test(a.eventos()) && /FW distinto/i.test(a.eventos()) &&
           /amber/.test(a.colorDe(/FW distinto/i)),
      'Grupos: aviso (c) FW distinto entre los dos postes, en ambar y con los dos sellos');
    a.status('ESCLAVO', 'SEM-E-99');
    assert(/serie/i.test(a.eventos()) && /SEM-E-99/.test(a.eventos()) && a.de('PR 45').ESCLAVO.serie === 'SEM-E-01',
      'Grupos: aviso (c) serie del $STATUS distinta de la guardada, y no se sobrescribe');
    a.status('MAESTRO', 'SEM-E-01');
    assert(/NODE:MAESTRO/.test(a.eventos()) && /red/.test(a.colorDe(/NODE:MAESTRO/)) &&
           a.de('PR 45').ESCLAVO.mac === MAC2 && a.de('PR 45').MAESTRO.mac === MAC1,
      'Grupos: aviso (c) NODE: distinto del hueco, en rojo, y la matricula no cambia');
    a.enlazar({ name: 'SEM-E-01-E', id: MAC3, address: MAC3 }, 'ESCLAVO', 'SEM-E-01');
    assert(/m[oó]dulo Bluetooth cambiado/i.test(a.eventos()) && /amber/.test(a.colorDe(/Bluetooth cambiado/i)) &&
           /SIN GRUPO/.test(a.sitio()),
      `Grupos: aviso (c) serie matriculada por otra MAC, en ambar, y la MAC nueva «Sin grupo»: "${a.sitio()}"`);

    // 7. Lista de equipos agrupada por via y grupo; las no vistas en gris; «Sin grupo» al final; busqueda.
    a.w.bluetoothSerial.list = (ok) => ok([{ name: 'SEM-M-01-M', id: MAC1, address: MAC1 },
                                           { name: 'SEM-ZZ7-M', id: MAC4, address: MAC4 }]);
    a.el('btnDevice').click();
    const caja = a.el('bt-device-list-container');
    const t = caja.textContent;
    assert(t.indexOf('Via 2') >= 0 && t.indexOf('Via 2') < t.indexOf('PR 45') && t.indexOf('PR 45') < t.indexOf('Sin grupo') &&
           t.indexOf('Sin grupo') < t.indexOf('SEM-ZZ7'),
      `Grupos: la lista de equipos va por via y grupo, y «Sin grupo» al final (${t.replace(/\s+/g, ' ').slice(0, 160)})`);
    const gris = caja.querySelector('.bt-no-visto');
    assert(!!gris && gris.textContent.includes(MAC2) && !gris.classList.contains('bt-device-item'),
      'Grupos: el poste matriculado que no se ve en esta busqueda sale en gris, sin pulsar');
    const filtro = a.el('bt-filtro-equipos');
    const filas = () => Array.from(caja.querySelectorAll('.bt-device-item')).map(f => f.getAttribute('data-mac'));
    if (filtro) { filtro.value = 'zz7'; filtro.dispatchEvent(new a.w.Event('input', { bubbles: true })); }
    const soloZ = filas();
    if (filtro) { filtro.value = 'PR 45'; filtro.dispatchEvent(new a.w.Event('input', { bubbles: true })); }
    const soloG = filas();
    assert(!!filtro && soloZ.join() === MAC4 && soloG.join() === MAC1,
      `Grupos: la busqueda filtra por serie y por grupo (${soloZ.join()} / ${soloG.join()})`);
  }

  // 8. Transporte sin `address` (Web Bluetooth, puerto serie del PC): no matricula y la ficha dice por que.
  {
    const a = montar();
    a.crear('Via 1', 'Km 120', '');
    a.clave();
    a.enlazar({ name: 'SEM-WB-M', id: 'wb-Zk3d9Q==' }, 'MAESTRO', 'SEM-WB');
    const f = a.ficha('Km 120');
    const lista = a.el('dynamic-site-list').textContent;
    assert(!!f && !a.d.querySelector('#dynamic-site-list .btn-matricular') && /no da la direcci[oó]n/i.test(lista) &&
           /SIN GRUPO/.test(a.sitio()),
      `Grupos: un enlace sin address no matricula, sale «SIN GRUPO» y lo dice: "${lista.replace(/\s+/g, ' ').slice(0, 120)}"`);
  }

  // 8b. La serie es la que dijo ESTE enlace: un $STATUS sin SERIE: tras cambiar de poste no hereda la del anterior.
  {
    const a = montar();
    a.crear('Via 1', 'Km 121', '');
    a.clave();
    a.enlazar({ name: 'SEM-E-05-E', id: MAC2, address: MAC2 });
    a.trama('STATUS,NODE:ESCLAVO,MODO:AUTO,ESTADO:V1_R2,T:30,RF:97,RTT:70,BAT:12.9,HORA:14:31:00');
    const f = a.ficha('Km 121');
    assert(!!f && !f.querySelector('.btn-matricular'),
      'Grupos: sin SERIE: de este poste no se ofrece Matricular con la serie del poste anterior');
  }

  // 9. Migracion de iot_vial_cruces_v2: el de fabrica intacto no pasa; el renombrado y el nuevo, a «Sin via».
  {
    const viejo = JSON.stringify([
      { id: 'cruce-1', nombre: '📍 Cruce Km 12 · El Sisga', ubicacion: 'PR 12+400 Ruta Nacional 55' },
      { id: 'cruce-2', nombre: 'Km 45 renombrado', ubicacion: 'PR 45+100 Frente Túnel 2' },
      { id: 'cruce-77', nombre: 'Obra Guaduas', ubicacion: 'PR 62+300' }
    ]);
    const a = montar({ iot_vial_cruces_v2: viejo });
    const g = a.guardado();
    assert(g.length === 2 && g.every(x => x.via === 'Sin via' && !x.MAESTRO && !x.ESCLAVO) &&
           g.some(x => x.nombre === 'Km 45 renombrado') && g.some(x => x.nombre === 'Obra Guaduas' && x.pr === 'PR 62+300'),
      `Grupos: migracion, el de fabrica no pasa y los otros dos van sin postes a «Sin via» (${g.map(x => x.nombre).join(', ')})`);
    assert(a.w.localStorage.getItem('iot_vial_cruces_v2') === viejo, 'Grupos: la clave vieja no se borra');
  }

  // 10. El modelo rechaza tambien sin la interfaz, y ningun rechazo escribe en el almacen.
  {
    const G = require('../js/grupos.js');
    let t = null;
    const gr = G.nuevo({ leer: () => t, escribir: (x) => { t = x; } });
    const g1 = gr.crear('V', 'A', ''), g2 = gr.crear('V', 'B', '');
    const p1 = { mac: MAC1, serie: 'S1', node: 'MAESTRO' };
    gr.matricular(g1.id, p1);
    const antes = t;
    const r = [gr.matricular(g1.id, { mac: MAC2, serie: 'S2', node: 'MAESTRO' }).motivo, gr.matricular(g2.id, p1).motivo,
               gr.matricular(g2.id, { mac: 'SERIAL_PC', serie: 'S3', node: 'ESCLAVO' }).motivo];
    assert(r.join() === 'OCUPADO,EN_OTRO_GRUPO,SIN_DIRECCION' && t === antes,
      `Grupos: el modelo rechaza hueco ocupado, MAC en otro grupo y transporte sin MAC, sin escribir (${r.join()})`);
  }

  // 11. state.site es el del poste ENLAZADO (9.14 (b)): al desconectar y al elegir otro poste, «SIN GRUPO» hasta que
  //     hable; nombra los exportados y el «Cruce:» del informe, que no pueden salir con el grupo de otro poste.
  {
    const a = montar();
    a.crear('Via 1', 'Km 20', '');
    a.clave();
    const b = a.boton('Km 20', '.btn-matricular');
    if (b) b.click();
    const s1 = a.sitio();
    a.el('btn-bt-disconnect').click();
    const s2 = a.sitio();
    a.enlazar({ name: 'SEM-M-02-M', id: MAC2, address: MAC2 });
    const s3 = a.sitio();
    assert(/Via 1 · Km 20/.test(s1) && /SIN GRUPO/.test(s2) && /SIN GRUPO/.test(s3),
      `Grupos: state.site vuelve a «SIN GRUPO» al desconectar ("${s2}") y al elegir otro poste ("${s3}"); antes "${s1}"`);
  }
};
