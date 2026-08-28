// ===== js/courier_rtc.js =====
// Asistente Courier RTC: Sincronización Horaria y de Ciclo sin Enlace Radio

const CourierRTC = {
  /**
   * Captura la hora y estado del Maestro
   */
  capturarMaestro(horaStr, faseActual, tiempoRestanteSeg) {
    const timestampCaptura = Date.now();
    return {
      timestampCaptura,
      horaStr,
      faseActual,
      tiempoRestanteSeg: parseInt(tiempoRestanteSeg, 10) || 0
    };
  },

  /**
   * Calcula la hora y fase compensada tras el tiempo de traslado
   */
  calcularCompensacion(snapshot, timestampInyeccion = Date.now()) {
    if (!snapshot || !snapshot.timestampCaptura) {
      throw new Error('Snapshot de Maestro inválido');
    }

    const elapsedMs = timestampInyeccion - snapshot.timestampCaptura;
    if (elapsedMs < 0) {
      throw new Error('Inconsistencia temporal: el tiempo de inyección es anterior a la captura');
    }

    const elapsedSeg = Math.floor(elapsedMs / 1000);
    
    // Parsear hora capturada HH:MM:SS
    const [hh, mm, ss] = snapshot.horaStr.split(':').map(Number);
    const dateObj = new Date();
    dateObj.setHours(hh || 0, mm || 0, ss || 0, 0);
    dateObj.setSeconds(dateObj.getSeconds() + elapsedSeg);

    const horaCompensada = dateObj.toTimeString().split(' ')[0]; // HH:MM:SS
    const dateStr = `${dateObj.getFullYear()}-${String(dateObj.getMonth() + 1).padStart(2, '0')}-${String(dateObj.getDate()).padStart(2, '0')}`;

    return {
      elapsedSeg,
      horaCompensada,
      dateStr,
      faseOriginal: snapshot.faseActual
    };
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = CourierRTC;
}
