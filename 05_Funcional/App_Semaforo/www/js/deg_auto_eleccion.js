// ===== js/deg_auto_eleccion.js =====
// LA ELECCION DEL DEGRADADO AUTOMATICO, DICHA EN EL MOMENTO (encargo C, decision del responsable).
// El funcional no lee manuales: la app dice que esta elegido, pide elegir si no consta y,
// al caer la radio, que va a pasar. No manda nada por si misma: las ordenes salen por
// DegAuto._pulsar() (PIN, espera del $ACK diferido) y el estado es el de CONSULTA_DEG_AUTO.
//
//   linea      pantalla principal: "Si se cae la radio: ..." y, si ESTE y OTRO difieren,
//              "Los postes no coinciden". OTRO es el ultimo APTO oido del otro poste
//              (respaldo_otroApto() del firmware), no su opcion: lo dice la tarjeta.
//   pide       al conectar, si la consulta no contesta o ESTE y OTRO difieren. Se pospone
//              hasta la siguiente conexion (DegAuto.alStatus llama a olvidar()).
//   sin radio  ESTADO:FALLO COM fuera de MODO:AMBAR, o RF medido al 0 %. Con la eleccion
//              en (a): ambar y testigo. En (b): cuenta aprox. desde que la APP vio caer
//              la radio; el firmware cuenta desde el ultimo latido, antes. Con el
//              $EVENT AUTO_ENTRADA_INICIO o MODO:DEGRADADO: "En Degradado por reloj".

const DegAutoEleccion = {
  // DEG_AUTO_ESPERA_MS de Maestro/include/deg_auto.h y Esclavo/include/deg_auto.h (300000UL).
  // Si cambia alli, cambia aqui y en las etiquetas "5 min" de DegAuto y de esta linea.
  ESPERA_MS: 300000,
  NO_CONSTA_MS: 10000,        // tras la consulta de DegAuto, sin respuesta: la eleccion no consta
  LINEA_OFF: 'Si se cae la radio: AMBAR, ir con testigo',
  LINEA_ON: 'Si se cae la radio: DEGRADADO SOLO a los 5 min',
  NO_COINCIDEN: 'Los postes no coinciden',
  CARTEL_OFF: 'Sin radio. Los postes quedan en AMBAR intermitente y NO arrancan solos: hay que ir ' +
              'con el testigo (Degradado con testigo)',
  MOTIVO_DISTINTOS: 'Los postes no tienen la misma eleccion',
  MOTIVO_NO_APTO: 'Este poste no esta listo para entrar solo',
  MARGEN_MS: 60000,           // retraso con que la app ve FALLO COM respecto al reloj del firmware
  CARTEL_NO_ENTRARON: 'No entraron solos: quedan en AMBAR. Hay que ir con el testigo',
  CARTEL_DEG: 'En Degradado por reloj, sin radio',

  ctx: null,
  _el: {},
  _conectadoMs: null,         // primer render con poste identificado
  _pospuesto: false,
  _caidaMs: null,             // cuando la app vio caer la radio
  _entrada: false,            // llego AUTO_ENTRADA_INICIO

  sinRadio(s) {
    if (!s || !s.node || s.telemetriaViva === false) return false;
    if (s.estadoLuces === 'FALLO COM' && s.modo !== 'AMBAR') return true;
    const l = s.rfLectura;
    return !!l && l.medido === true && l.pct === 0;
  },

  olvidar() {
    this._conectadoMs = null; this._pospuesto = false; this._caidaMs = null; this._entrada = false;
  },

  entrada() { this._entrada = true; this.render(); },

  iniciar(ctx) {
    this.ctx = ctx;
    const $ = (id) => document.getElementById(id);
    this._el = { linea: $('degauto-linea'), lineaTxt: $('degauto-linea-txt'), difieren: $('degauto-linea-difieren'),
                 pide: $('degauto-pide'), pideMotivo: $('degauto-pide-motivo'), pideNota: $('degauto-pide-sin-radio'),
                 pideOff: $('btn-degauto-pide-off'), pideOn: $('btn-degauto-pide-on'), luego: $('btn-degauto-pide-luego'),
                 cartel: $('degauto-sinradio'), cartelTxt: $('degauto-sinradio-txt'), irTestigo: $('btn-degauto-ir-testigo') };
    const el = this._el;
    const elegir = (v) => { this._pospuesto = true; DegAuto._pulsar(v); this.render(); };
    if (el.pideOff) el.pideOff.addEventListener('click', () => elegir('0'));
    if (el.pideOn) el.pideOn.addEventListener('click', () => elegir('1'));
    if (el.luego) el.luego.addEventListener('click', () => { this._pospuesto = true; this.render(); });
    if (el.irTestigo) el.irTestigo.addEventListener('click', () => this.irATestigo());
  },

  // La tarjeta del testigo esta en la pestana Diagnostico, que solo ve el Tecnico.
  irATestigo() {
    const nav = document.querySelector('.nav-item[data-tab="tab-diag"]');
    if (!nav || nav.style.display === 'none') {
      this.ctx.showToast('Entre como Tecnico (boton de arriba) y abra Diagnostico: Degradado con testigo');
      return;
    }
    nav.click();
    const card = document.getElementById('card-testigo');
    if (card && card.scrollIntoView) card.scrollIntoView();
  },

  _mmss(ms) {
    const s = Math.max(0, Math.ceil(ms / 1000));
    return Math.floor(s / 60) + ':' + String(s % 60).padStart(2, '0');
  },

  // Texto del cartel sin radio, o null si no toca.
  textoCartel(s, e, ahora) {
    const sinRadio = this.sinRadio(s);
    if (s.modo === 'DEGRADADO' && (this._entrada || this._caidaMs !== null)) return this.CARTEL_DEG;
    if (!sinRadio) { this._caidaMs = null; this._entrada = false; return null; }
    if (this._caidaMs === null) this._caidaMs = ahora;
    if (!e) return 'Sin radio. No consta que hacen los postes sin radio: mire el modo de cada poste.';
    // Cuenta solo si el firmware puede entrar: degAuto_loop() pide respaldo_otroApto() (OTRO) y
    // puertaAbierta(), y entrar() repite lo de degAuto_aptoPropio() (APTO). Si no, se queda en ambar.
    if (e.este !== e.otro) return this.CARTEL_OFF + '. Motivo: ' + this.MOTIVO_DISTINTOS;
    if (!e.este) return this.CARTEL_OFF;
    if (!e.apto) return this.CARTEL_OFF + '. Motivo: ' + this.MOTIVO_NO_APTO;
    // El ECO (respaldo_aptoDado()) no llega a la app: si la cuenta vence y pasa el margen sin
    // AUTO_ENTRADA_INICIO ni MODO:DEGRADADO, no entraron.
    const resta = this.ESPERA_MS - (ahora - this._caidaMs);
    if (!this._entrada && resta < -this.MARGEN_MS) return this.CARTEL_NO_ENTRARON;
    return 'Sin radio. Entran SOLOS en Degradado en aprox. ' + this._mmss(resta);
  },

  render() {
    const el = this._el;
    if (!this.ctx) return;
    const s = this.ctx.state;
    const e = DegAuto._estado;
    const ahora = Date.now();
    const conectado = !!s.node && s.telemetriaViva !== false;
    if (!conectado) this._conectadoMs = null;
    else if (this._conectadoMs === null) this._conectadoMs = ahora;
    const difieren = !!e && typeof e.otro === 'boolean' && e.este !== e.otro;

    if (el.linea) {
      el.linea.hidden = !conectado;
      if (el.lineaTxt) {
        el.lineaTxt.textContent = DegAuto._sinOpcion ? 'Si se cae la radio: este firmware no trae la eleccion'
          : !e ? 'Si se cae la radio: consultando...' : (e.este ? this.LINEA_ON : this.LINEA_OFF);
      }
      if (el.difieren) el.difieren.hidden = !difieren;
    }

    if (el.pide) {
      const noConsta = !e && !DegAuto._sinOpcion && this._conectadoMs !== null &&
                       ahora - this._conectadoMs > DegAuto.CONSULTA_TRAS_MS + this.NO_CONSTA_MS;
      el.pide.hidden = !conectado || this._pospuesto || DegAuto._sinOpcion || !(difieren || noConsta);
      const sinRadio = this.sinRadio(s);
      if (el.pideMotivo) {
        el.pideMotivo.textContent = difieren
          ? this.NO_COINCIDEN + ': este poste ' + (e.este ? 'ON' : 'OFF') + ', el otro ' + (e.otro ? 'ON' : 'OFF') + '.'
          : 'El poste no ha dicho que tiene elegido.';
      }
      if (el.pideNota) el.pideNota.hidden = !sinRadio;
      [el.pideOff, el.pideOn].forEach(b => { if (b) b.disabled = sinRadio || !!DegAuto._espera; });
    }

    if (el.cartel) {
      const t = conectado ? this.textoCartel(s, e, ahora) : null;
      el.cartel.hidden = !t;
      if (el.cartelTxt) el.cartelTxt.textContent = t || '';
      if (el.irTestigo) el.irTestigo.hidden = !t || (t.indexOf(this.CARTEL_OFF) !== 0 && t !== this.CARTEL_NO_ENTRARON);
    }
  },
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = DegAutoEleccion;
}
