// ===== js/pantalla_operario.js =====
// N-181 U (SPEC_4 9.9 (f)-(g); D-141..D-144): lo que la pantalla del operario resume, pliega u ofrece.
// - #resumen-equipo, UNA linea, se escribe con lo que YA pintaron las tres tarjetas de «Detalle»: los escritores de
//   cada dato siguen siendo pintarEnlace(), la bateria del $STATUS y pintarCamaras(); este solo resume, con el color
//   de D-83 por trozo y gris sin dato. Sin «antena» ni «senal»: la nota de 9.9 (a) 3 va junto a su barra.
// - «Averias» (D-94) se abre y va en ambar si la barrera o las camaras estan fuera (PLUMA:FUERA, CAM:NOVETO; D-97).
// - El aviso «sin emparejar» ofrece «Emparejar y conectar», que repite el enlace con la MISMA MAC (D-144).
const PantallaOperario = {
  RADIO: { 'enlace-bien': ['BUENA', 'res-verde'], 'enlace-justo': ['JUSTA', 'res-ambar'],
           'enlace-cayendo': ['CAYENDO', 'res-rojo'] },
  CAM: { 'OK': ['bien', 'res-verde'], 'SIN COMPROBAR': ['sin comprobar', 'res-sindato'], 'CIEGA': ['ciega', 'res-ambar'],
         'PEGADA': ['pegada', 'res-rojo'], 'SIN VETO': ['sin veto', 'res-ambar'], '!': ['!', 'res-rojo'] },

  // estado: funcion que devuelve el state de app.js (solo se leen pluma y cam).
  instalar(estado) {
    this._estado = estado;
    const ver = () => this.resumir();
    ['rf-estado', 'bat-voltage', 'cam-estado', 'pluma-estado', 'bt-btn-text'].forEach(id => {
      const el = document.getElementById(id);
      if (el) new MutationObserver(ver).observe(el, { childList: true, characterData: true, subtree: true, attributes: true });
    });
    const b = document.getElementById('btn-aviso-pareja');
    if (b) b.addEventListener('click', () => { const r = this._reintento; this.sinPareja(null); if (r) r(); });
    ver();
  },

  resumir() {
    const $ = (id) => document.getElementById(id), res = $('resumen-equipo');
    if (!res) return;
    const st = this._estado ? this._estado() : {}, vivo = st.telemetriaViva !== false;   // sin enlace, nada viejo
    const rf = vivo && $('rf-estado'), bat = vivo && $('bat-voltage'), cam = vivo && $('cam-estado');
    const radio = (rf && this.RADIO[Array.from(rf.classList).find(c => this.RADIO[c])]) || ['sin dato', 'res-sindato'];
    const v = bat ? bat.textContent.replace(/\s*V\s*$/, '').trim() : '--';
    const c = (cam && this.CAM[cam.textContent.trim()]) || ['--', 'res-sindato'];
    res.textContent = '';
    [['Radio entre postes: ' + radio[0], radio[1]], ['Batería ' + v + ' V', /\d/.test(v) ? 'res-verde' : 'res-sindato'],
     ['Cámaras ' + c[0], c[1]]].forEach(([t, cl], k) => {
      if (k) res.appendChild(document.createTextNode(' · '));
      const s = document.createElement('span');
      s.className = cl; s.textContent = t; res.appendChild(s);
    });
    const fuera = st.pluma === 'FUERA' || st.cam === 'NOVETO', av = $('averias');
    if (av) { if (fuera) av.open = true; av.querySelector('summary').classList.toggle('averia-fuera', fuera); }
  },

  // reintento: la funcion que vuelve a abrir el enlace con la misma MAC; null retira el aviso.
  sinPareja(reintento) {
    this._reintento = reintento;
    const el = document.getElementById('aviso-pareja');
    if (el) el.hidden = !reintento;
  },

  // 9.9 (g) 1: con el Esclavo «Que se ve desde este poste» ocupa el sitio de «Detalle».
  verDetalle(si) {
    const d = document.getElementById('detalle-equipo');
    if (d) d.hidden = !si;
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = PantallaOperario;
}
