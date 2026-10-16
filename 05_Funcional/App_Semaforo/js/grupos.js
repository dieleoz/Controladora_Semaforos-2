// ===== js/grupos.js =====
// GRUPOS POR VIA Y MATRICULA POR MAC (SPEC_4 9.14 (a)..(e); D-117, D-118, D-119, D-123). Sustituye a
// js/site_manager.js y sus «cruces» de texto libre. Una via tiene grupos (Km 20, PR 45); cada grupo, dos huecos,
// MAESTRO y ESCLAVO, con {mac, serie, fw, fecha}. La identidad del poste es la MAC (`address` de bt.list()); serie y
// punta salen del $STATUS (SERIE:, NODE:) y el FW del `FW:` de la respuesta a VERSION. Nada de eso se teclea.
// Matricular es control del operador: NO envia nada al equipo y no cambia la radio (SPEC 6 B.1 (4), B.2).
// El almacen se inyecta, `{ leer(): string|null, escribir(texto) }`: la app le da uno sobre su almacen del navegador
// y la practica uno en memoria. Este fichero no toca el almacen del navegador por su cuenta.
const Grupos = (() => {
  const CLAVE = 'iot_vial_grupos_v1';
  const CLAVE_VIEJA = 'iot_vial_cruces_v2';   // la de js/site_manager.js (retirado, D-119); se lee una vez, no se borra
  const SIN_VIA = 'Sin via';
  const SIN_GRUPO = 'SIN GRUPO';
  const PUNTAS = ['MAESTRO', 'ESCLAVO'];
  // Tope de cortesia del nombre; la garantia de que la cabecera no se parte es el truncado de .site-name en el CSS.
  const MAX_NOMBRE = 32;
  // Los dos CRUCES_DEFAULT de site_manager.js, solo para reconocerlos al migrar: un cruce es de fabrica si id,
  // nombre y ubicacion son estos sin tocar (D-123). En listas y no en objetos: la app ya no trae grupos de fabrica.
  const DE_FABRICA = [
    ['cruce-1', '📍 Cruce Km 12 · El Sisga', 'PR 12+400 Ruta Nacional 55'],
    ['cruce-2', '📍 Km 45 · Vía al Llano', 'PR 45+100 Frente Túnel 2']
  ];
  const RADIO = 'La radio entre los dos postes la fija su canal (SPEC 6 B.2): la app no la ve. El grupo es el ' +
                'control del operador, no un enlace.';
  // Solo una direccion con forma de MAC matricula. Web Bluetooth da un `id` opaco y el puerto serie del PC un
  // rotulo fijo (js/serial_pc.js): con eso no hay identidad del poste (D-123).
  const esMac = (m) => /^([0-9A-F]{2}:){5}[0-9A-F]{2}$/i.test(String(m || ''));
  const recortar = (t) => { const n = String(t || '').trim(); return n.length > MAX_NOMBRE ? n.slice(0, MAX_NOMBRE).trim() : n; };
  const otra = (p) => (p === 'MAESTRO' ? 'ESCLAVO' : 'MAESTRO');
  let seq = 0;

  function migrar(textoViejo) {
    let cruces = null;
    try { cruces = JSON.parse(textoViejo); } catch (e) { return []; }
    if (!Array.isArray(cruces)) return [];
    return cruces.filter(c => c && !DE_FABRICA.some(f => f[0] === c.id && f[1] === c.nombre && f[2] === c.ubicacion))
      .map((c, i) => ({ id: 'g-migrado-' + i, via: SIN_VIA,
                        nombre: recortar(String(c.nombre || '').replace('📍', '')) || 'Cruce sin nombre',
                        pr: String(c.ubicacion || '').trim(), MAESTRO: null, ESCLAVO: null }));
  }

  // Una instancia por almacen. `textoViejo`: lo que habia en CLAVE_VIEJA; solo se usa si el almacen esta vacio.
  function nuevo(almacen, textoViejo) {
    let datos = null;
    const guardar = () => { try { almacen.escribir(JSON.stringify(datos)); } catch (e) { /* sin almacen: dura la sesion */ } };
    function cargar() {
      if (datos) return datos;
      let t = null;
      try { t = almacen.leer(); } catch (e) { t = null; }
      if (t === null || t === undefined) {
        datos = { v: 1, grupos: textoViejo ? migrar(textoViejo) : [] };
        guardar();
        return datos;
      }
      try { datos = JSON.parse(t); } catch (e) { datos = null; }
      if (!datos || !Array.isArray(datos.grupos)) datos = { v: 1, grupos: [] };   // ilegible: no se pisa hasta un cambio
      return datos;
    }
    const copia = (x) => JSON.parse(JSON.stringify(x));
    const buscar = (id) => cargar().grupos.find(g => g.id === id) || null;
    const rotuloDe = (g) => g.via + ' · ' + g.nombre;

    function dondeEsta(mac) {
      if (!mac) return null;
      for (const g of cargar().grupos) {
        for (const p of PUNTAS) if (g[p] && g[p].mac === mac) return { grupo: g, punta: p };
      }
      return null;
    }

    function crear(via, nombre, pr) {
      const n = recortar(nombre);
      if (!n) return null;
      const g = { id: 'g-' + Date.now() + '-' + (++seq), via: recortar(via) || SIN_VIA, nombre: n,
                  pr: String(pr || '').trim(), MAESTRO: null, ESCLAVO: null };
      cargar().grupos.push(g);
      guardar();
      return copia(g);
    }

    function renombrar(id, nombre) {
      const g = buscar(id), n = recortar(nombre);
      if (!g || !n) return false;
      g.nombre = n;
      guardar();
      return true;
    }

    // Un grupo con postes no se borra: primero «Quitar del grupo», que es lo que deja constancia de la baja.
    function borrar(id) {
      const g = buscar(id);
      if (!g || g.MAESTRO || g.ESCLAVO) return false;
      datos.grupos = datos.grupos.filter(x => x !== g);
      guardar();
      return true;
    }

    // poste = { mac, serie, node, fw }. La punta es la del NODE: del $STATUS. `mover` = el operador ya confirmo.
    // Devuelve { ok } o { ok:false, motivo, grupo? }; ningun rechazo cambia el almacen.
    function matricular(id, poste, mover) {
      const p = poste || {};
      if (!esMac(p.mac)) return { ok: false, motivo: 'SIN_DIRECCION' };
      if (!p.serie || PUNTAS.indexOf(p.node) < 0) return { ok: false, motivo: 'SIN_STATUS' };
      const g = buscar(id);
      if (!g) return { ok: false, motivo: 'SIN_GRUPO' };
      if (g[p.node] && g[p.node].mac !== p.mac) return { ok: false, motivo: 'OCUPADO' };
      const antes = dondeEsta(p.mac);
      if (antes && antes.grupo !== g && !mover) return { ok: false, motivo: 'EN_OTRO_GRUPO', grupo: copia(antes.grupo) };
      if (antes) antes.grupo[antes.punta] = null;
      g[p.node] = { mac: p.mac, serie: p.serie, fw: p.fw || null, fecha: new Date().toISOString().slice(0, 10) };
      guardar();
      return { ok: true };
    }

    function quitar(mac) {
      const d = dondeEsta(mac);
      if (!d) return false;
      d.grupo[d.punta] = null;
      guardar();
      return true;
    }

    // El sello del hueco es el `FW:` que conteste el propio poste a VERSION, tambien SIN_SELLAR, tal cual (D-123).
    function anotarFw(mac, node, fw) {
      const d = dondeEsta(mac);
      if (!d || d.punta !== node || fw === undefined || d.grupo[d.punta].fw === fw) return false;
      d.grupo[d.punta].fw = fw;
      guardar();
      return true;
    }

    // Los cuatro avisos de (c), solo con lo que dice el equipo. Ninguno cambia la matricula.
    function avisos(poste) {
      const p = poste || {};
      const out = [];
      if (!p.mac) return out;
      const d = dondeEsta(p.mac);
      if (d) {
        const h = d.grupo[d.punta], r = rotuloDe(d.grupo);
        if (p.serie && h.serie !== p.serie) {
          out.push({ color: 'red', texto: 'Serie cambiada en ' + r + ': el equipo dice ' + p.serie + ' y se matriculó ' +
                     'con ' + h.serie + '. No se sobrescribe: matricule otra vez si es el poste correcto.' });
        }
        if (p.node && p.node !== d.punta) {
          out.push({ color: 'red', texto: 'El equipo dice NODE:' + p.node + ' y está matriculado como ' + d.punta +
                     ' de ' + r + '. La matrícula no cambia: revise qué poste es.' });
        }
        const o = d.grupo[otra(d.punta)];
        if (h.fw && o && o.fw && h.fw !== o.fw) {
          out.push({ color: 'amber', texto: 'FW distinto en ' + r + ': ' + d.punta + ' ' + h.fw + ', ' + otra(d.punta) +
                     ' ' + o.fw + ' (D-102).' });
        }
      } else if (p.serie) {
        for (const g of cargar().grupos) {
          for (const q of PUNTAS) {
            if (g[q] && g[q].serie === p.serie && g[q].mac !== p.mac) {
              out.push({ color: 'amber', texto: 'La serie ' + p.serie + ' ya está matriculada en ' + rotuloDe(g) +
                         ' con la dirección ' + g[q].mac + ': ¿módulo Bluetooth cambiado? Esta dirección queda ' +
                         '«Sin grupo» hasta matricularla.' });
            }
          }
        }
      }
      return out;
    }

    const rotulo = (mac) => { const d = dondeEsta(mac); return d ? rotuloDe(d.grupo) : SIN_GRUPO; };

    function filtrar(q) {
      const t = String(q || '').toLowerCase().trim();
      const todos = cargar().grupos.map(copia);
      if (!t) return todos;
      return todos.filter(g => [g.via, g.nombre, g.pr].concat(PUNTAS.map(p => (g[p] ? g[p].serie : '')))
        .some(x => String(x || '').toLowerCase().includes(t)));
    }

    return { crear, renombrar, borrar, matricular, quitar, anotarFw, avisos, rotulo, filtrar,
             todos: () => cargar().grupos.map(copia), dondeEsta: (mac) => { const d = dondeEsta(mac); return d ? copia(d) : null; } };
  }

  // --- La ficha de cada grupo, en el modal de grupos (#dynamic-site-list) ---
  // ctx = { poste: {mac, serie, node, fw} | null, admin: bool, alCambiar(), avisar(color, texto) }.
  function el(tag, clase, texto) {
    const e = document.createElement(tag);
    if (clase) e.className = clase;
    if (texto !== undefined) e.textContent = texto;
    return e;
  }
  function boton(clase, texto, alPulsar) {
    const b = el('button', 'btn-top ' + clase, texto);
    b.style.cssText = 'padding: 4px 8px; font-size: 10px;';
    b.addEventListener('click', (ev) => { ev.stopPropagation(); alPulsar(); });
    return b;
  }

  function notaDelPoste(gr, ctx) {
    const p = ctx.poste;
    if (!p || !p.mac) return '';
    if (!esMac(p.mac)) {
      return 'Este enlace no da la dirección MAC del módulo (Web Bluetooth o puerto serie del PC): el poste sale ' +
             '«SIN GRUPO» y no se puede matricular. Matricule desde la APK.';
    }
    const d = gr.dondeEsta(p.mac);
    if (d) return 'Poste enlazado: ' + d.punta + ' de ' + d.grupo.via + ' · ' + d.grupo.nombre + '.';
    if (!p.serie || PUNTAS.indexOf(p.node) < 0) return 'Poste enlazado SIN GRUPO: esperando su primer $STATUS.';
    if (!ctx.admin) return 'Poste enlazado SIN GRUPO (' + p.serie + ', ' + p.node + '). Matricular pide la clave de ' +
                           'administración.';
    return 'Poste enlazado SIN GRUPO: ' + p.serie + ' (' + p.node + '), dirección ' + p.mac + '. Pulse «Matricular ' +
           'aquí» en un grupo con el hueco ' + p.node + ' libre, o cree un «Grupo nuevo».';
  }

  function intentarMatricular(gr, g, ctx) {
    const r = gr.matricular(g.id, ctx.poste, false);
    if (r.ok) { ctx.avisar('cyan', 'Poste ' + ctx.poste.serie + ' matriculado como ' + ctx.poste.node + ' de ' +
                           g.via + ' · ' + g.nombre + '. No se ha enviado nada al equipo.'); ctx.alCambiar(); return; }
    if (r.motivo !== 'EN_OTRO_GRUPO') { ctx.avisar('red', 'No se matricula (' + r.motivo + ').'); return; }
    Dialogo.confirmar('Mover el poste', 'Este poste está matriculado en ' + r.grupo.via + ' · ' + r.grupo.nombre +
                      '. Una MAC va en un solo grupo: ¿moverlo a ' + g.via + ' · ' + g.nombre + '?', 'Mover', () => {
      if (gr.matricular(g.id, ctx.poste, true).ok) ctx.alCambiar();
    });
  }

  function ficha(gr, g, ctx) {
    const item = el('div', 'bt-device-item site-card');
    item.style.cssText = 'margin-bottom: 8px; flex-wrap: wrap;';
    const info = el('div', 'bt-dev-info');
    info.appendChild(el('strong', 'site-card-title', g.nombre + (g.pr ? ' · ' + g.pr : '')));
    PUNTAS.forEach(p => {
      const h = g[p];
      const linea = el('small', 'hueco-' + p.toLowerCase(), h
        ? p + ': ' + h.serie + ' · Dirección ' + h.mac + ' · FW ' + (h.fw === null ? 'sin leer' : h.fw) + ' · ' + h.fecha
        : p + ': libre');
      info.appendChild(linea);
      if (h && ctx.admin) info.appendChild(boton('btn-quitar-hueco', 'Quitar del grupo', () => {
        if (gr.quitar(h.mac)) { ctx.avisar('cyan', p + ' ' + h.serie + ' quitado de ' + g.via + ' · ' + g.nombre + '.'); ctx.alCambiar(); }
      }));
    });
    info.appendChild(el('small', 'ficha-radio', RADIO));
    item.appendChild(info);
    const p = ctx.poste;
    if (ctx.admin && p && esMac(p.mac) && p.serie && PUNTAS.indexOf(p.node) >= 0 && !g[p.node]) {
      item.appendChild(boton('btn-matricular', 'Matricular aquí', () => intentarMatricular(gr, g, ctx)));
    }
    item.appendChild(boton('btn-edit-site', '✏️', () => {
      Dialogo.pedirTexto({ titulo: 'Renombrar el grupo', texto: 'Nombre nuevo:', valor: g.nombre }, (n) => {
        if (gr.renombrar(g.id, n)) ctx.alCambiar();
      });
    }));
    if (!g.MAESTRO && !g.ESCLAVO) item.appendChild(boton('btn-delete-site', '🗑️', () => { if (gr.borrar(g.id)) ctx.alCambiar(); }));
    return item;
  }

  function pintar(cont, gr, filtro, ctx) {
    if (!cont) return 0;
    cont.innerHTML = '';
    const nota = notaDelPoste(gr, ctx);
    if (nota) cont.appendChild(el('p', 'modal-desc nota-poste', nota));
    const lista = gr.filtrar(filtro).sort((a, b) => a.via.localeCompare(b.via) || a.nombre.localeCompare(b.nombre));
    let via = null;
    lista.forEach(g => {
      if (g.via !== via) { via = g.via; cont.appendChild(el('p', 'modal-desc grupo-via', '🛣️ ' + via)); }
      cont.appendChild(ficha(gr, g, ctx));
    });
    return lista.length;
  }

  // «Grupo nuevo»: via (opcional, «Sin via»), nombre y PR (opcional), en campos vacios (SPEC_4 9.1). Con la clave y
  // un poste enlazado sin grupo, se matricula en el grupo recien creado: es lo que el operador acaba de pedir.
  function preguntarNuevo(gr, ctx) {
    Dialogo.pedirTexto({ titulo: 'Grupo nuevo', texto: 'Vía (opcional):', opcional: true }, (via) => {
      Dialogo.pedirTexto({ titulo: 'Grupo nuevo', texto: 'Nombre del grupo (Km 20, PR 45):' }, (nombre) => {
        const fin = (pr) => {
          const g = gr.crear(via, nombre, pr);
          if (!g) return;
          const p = ctx.poste;
          if (ctx.admin && p && esMac(p.mac) && !gr.dondeEsta(p.mac)) gr.matricular(g.id, p, false);
          ctx.avisar('cyan', 'Grupo ' + g.via + ' · ' + g.nombre + ' guardado.');
          ctx.alCambiar();
        };
        Dialogo.pedirTexto({ titulo: 'Grupo nuevo', texto: 'PR (opcional):', opcional: true }, fin, () => fin(''));
      });
    });
  }

  return { CLAVE, CLAVE_VIEJA, SIN_VIA, SIN_GRUPO, PUNTAS, MAX_NOMBRE, RADIO, esMac, nuevo, migrar, pintar, preguntarNuevo };
})();

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Grupos;
}
