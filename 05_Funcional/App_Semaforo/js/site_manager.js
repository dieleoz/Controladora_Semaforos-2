// ===== js/site_manager.js =====
// Gestor de Frentes de Obra y Cruces Viales con Persistencia LocalStorage

const SiteManager = {
  STORAGE_KEY: 'iot_vial_cruces_v2',

  CRUCES_DEFAULT: [
    {
      id: 'cruce-1',
      nombre: '📍 Cruce Km 12 · El Sisga',
      ubicacion: 'PR 12+400 Ruta Nacional 55',
      p1: '👑 Maestro (Poste 1)',
      p2: '📡 Esclavo (Poste 2)'
    },
    {
      id: 'cruce-2',
      // N-75: 31 caracteres desbordaban la cabecera y partian el rotulo en dos
      // lineas, montandose con el RSSI. Se deja a la altura del otro, que si entra.
      nombre: '📍 Km 45 · Vía al Llano',
      ubicacion: 'PR 45+100 Frente Túnel 2',
      p1: '👑 Maestro (Poste 1)',
      p2: '📡 Esclavo (Poste 2)'
    }
  ],

  obtenerCruces() {
    try {
      const raw = localStorage.getItem(this.STORAGE_KEY);
      if (!raw) {
        this.guardarCruces(this.CRUCES_DEFAULT);
        return this.CRUCES_DEFAULT;
      }
      return JSON.parse(raw);
    } catch (e) {
      console.error('Error al leer cruces de LocalStorage:', e);
      return this.CRUCES_DEFAULT;
    }
  },

  guardarCruces(cruces) {
    try {
      localStorage.setItem(this.STORAGE_KEY, JSON.stringify(cruces));
    } catch (e) {
      console.error('Error al guardar cruces en LocalStorage:', e);
    }
  },

  // Tope de cortesia del nombre. La GARANTIA de que la cabecera no se rompe es el
  // truncado de .site-name en el CSS: un limite de caracteres no puede cubrir los
  // nombres que ya estan guardados en localStorage de versiones anteriores.
  MAX_NOMBRE: 32,

  recortar(nombre) {
    const n = (nombre || '').trim();
    return n.length > this.MAX_NOMBRE ? n.slice(0, this.MAX_NOMBRE).trim() : n;
  },

  agregarCruce(nombre, ubicacion, p1, p2) {
    const cruces = this.obtenerCruces();
    const nuevo = {
      // N-75: Date.now() a secas repite el id para dos altas en el mismo
      // milisegundo, y con editar y eliminar reconectados eso significa renombrar o
      // borrar OTRO cruce. El contador es monotono dentro de la sesion, asi que no
      // puede colisionar; el sello de tiempo lo separa entre sesiones. Nada de
      // Math.random(): un id al azar hace la prueba intermitente en vez de imposible.
      id: 'cruce-' + Date.now() + '-' + (SiteManager._seq = (SiteManager._seq || 0) + 1),
      nombre: this.recortar(nombre) || 'Nuevo Cruce Vial',
      ubicacion: ubicacion || 'Ubicación no especificada',
      p1: p1 || '👑 Maestro (P1)',
      p2: p2 || '📡 Esclavo (P2)'
    };
    cruces.push(nuevo);
    this.guardarCruces(cruces);
    return nuevo;
  },

  actualizarCruce(id, nombre, ubicacion, p1, p2) {
    nombre = this.recortar(nombre);
    const cruces = this.obtenerCruces();
    const idx = cruces.findIndex(c => c.id === id);
    if (idx !== -1) {
      cruces[idx] = { ...cruces[idx], nombre, ubicacion, p1, p2 };
      this.guardarCruces(cruces);
      return cruces[idx];
    }
    return null;
  },

  eliminarCruce(id) {
    let cruces = this.obtenerCruces();
    if (cruces.length <= 1) {
      throw new Error('No se puede eliminar el único cruce configurado');
    }
    const initialLen = cruces.length;
    cruces = cruces.filter(c => c.id !== id);
    if (cruces.length < initialLen) {
      this.guardarCruces(cruces);
      return true;
    }
    return false;
  },

  filtrarCruces(query) {
    const q = (query || '').toLowerCase().trim();
    if (!q) return this.obtenerCruces();
    return this.obtenerCruces().filter(c => 
      c.nombre.toLowerCase().includes(q) || 
      c.ubicacion.toLowerCase().includes(q)
    );
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = SiteManager;
}
