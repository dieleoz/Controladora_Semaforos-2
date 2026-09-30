// ===== js/aviso_corte.js =====
// N-170 (30/09): EL PARTE DE ARRANQUE DEL PUENTE ESP32 SE LEE, Y SI FUE UN CORTE SE AVISA.
//
// El ESP32 manda al conectarse un telefono (y otra vez en cada conexion: vigilante.cpp
// rearma el parte al caer el enlace):
//   $EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:<nombreCausa()>,ARRANQUES:n,PERRO:..,WDT_MS:..
// La rama $EVENT de app.js solo leia ORIGEN/DETALLE, que esta trama no trae, y en campo
// salio "Equipo [FIRMWARE]: " vacio (evidencia/300920261130).
//
// LO QUE LA TRAMA NO DICE: CUANDO. No trae hora, y como se repite en cada conexion el
// mismo corte sale cada vez que alguien se conecta, y tambien tras el primer encendido
// (roadmap 1.52). Por eso el texto dice "no se sabe cuando fue" y no inventa una hora.
//
// Una vez por conexion: ver() devuelve el aviso la primera vez y {repetido} despues;
// olvidar() lo rearma al soltar el enlace (olvidarEnlace() de app.js).

const AvisoCorte = {
  // Causas de nombreCausa() (ESP32_Expansion/src/vigilante.cpp). La que no este aqui
  // sale en crudo, que es la red y no el destino.
  CAUSAS: {
    SUBIDA_DE_TENSION:     'arranque por subida de tension',
    PIN_EXTERNO:           'reinicio por el pin de reset del modulo ESP32',
    REINICIO_POR_SOFTWARE: 'el firmware del ESP32 se reinicio a si mismo',
    EXCEPCION_O_PANICO:    'el firmware del ESP32 fallo (excepcion) y se reinicio',
    PERRO_DE_INTERRUPCION: 'el perro guardian del ESP32 lo reinicio (se habia colgado)',
    PERRO_DE_TAREAS:       'el perro guardian del ESP32 lo reinicio (se habia colgado)',
    OTRO_PERRO:            'el perro guardian del ESP32 lo reinicio (se habia colgado)',
    SUENO_PROFUNDO:        'salida de sueno profundo del ESP32',
    TENSION_BAJA:          'la alimentacion del modulo ESP32 bajo demasiado; revise la alimentacion',
    DESCONOCIDA:           'el ESP32 no sabe por que se reinicio'
  },
  TITULO: 'ESTE POSTE SE QUEDO SIN LUZ (arranque por subida de tension).',
  CUANDO: 'No se sabe cuando fue: la trama no trae la hora, sale en cada conexion ' +
          'y tambien tras el primer encendido.',
  ACCION: 'Revise el modo en que quedo y de la orden.',

  _visto: false,
  _cartel: false,

  esArranque(data) {
    return !!data && data.NODE === 'PUENTE' && data.EVT === 'ARRANQUE';
  },

  // null si no es el parte de arranque; {repetido:true} si ya se aviso en esta conexion;
  // si no, {tono, texto, toast?, corte}.
  ver(data) {
    if (!this.esArranque(data)) return null;
    if (this._visto) return { repetido: true };
    this._visto = true;
    const causa = data.CAUSA || '';
    const cuenta = data.ARRANQUES !== undefined ? ' Arranques contados: ' + data.ARRANQUES + '.' : '';
    if (causa === 'SUBIDA_DE_TENSION') {
      this._cartel = true;
      return { corte: true, tono: 'red',
               texto: this.TITULO + ' ' + this.CUANDO + ' ' + this.ACCION + cuenta,
               toast: 'ESTE POSTE SE QUEDO SIN LUZ - revise el modo en que quedo' };
    }
    const t = this.CAUSAS[causa];
    return { corte: false, tono: causa === 'TENSION_BAJA' ? 'red' : 'amber',
             texto: 'El modulo ESP32 de este poste arranco: ' +
                    (t || 'causa sin traducir (' + (causa || 'la trama no la trae') + ')') +
                    '. No se sabe cuando fue.' + cuenta };
  },

  // Texto del cartel, o null si no hay que ensenarlo. poste/modo: lo que dijo el
  // $STATUS de esta conexion, o null si todavia no llego.
  cartel(poste, modo) {
    if (!this._cartel) return null;
    const ahora = poste
      ? 'Ahora: ' + poste + (modo ? ', modo ' + modo : ', modo sin dato') + '.'
      : 'Todavia no ha llegado el estado del poste: espere a ver el modo antes de dar la orden.';
    return { titulo: this.TITULO, cuando: this.CUANDO, accion: this.ACCION + ' ' + ahora };
  },

  cerrar() { this._cartel = false; },
  olvidar() { this._visto = false; this._cartel = false; }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = AvisoCorte;
}
