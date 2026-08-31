// ===== js/courier_rtc.js =====
// Asistente Courier RTC: Sincronización Horaria y de Ciclo sin Enlace Radio

// LA HORA QUE ENTRA AQUI TIENE QUE SER HH:MM:SS DE 24 HORAS, Y SE EXIGE.
//
// Este modulo arrastraba en SILENCIO el mismo error de 12 h que app.js: el llamador
// capturaba con toLocaleTimeString(), que en el locale de campo (es-CO) devuelve
// "6:25:00 p. m." para las 18:25, y aqui se hacia
//
//     const [hh, mm, ss] = snapshot.horaStr.split(':').map(Number);
//
// MEDIDO en node: "6:25:00 p. m.".split(':') -> ["6", "25", "00 p. m."], y
// Number("00 p. m.") es NaN. NO REVENTABA: la linea de debajo era
// `dateObj.setHours(hh || 0, mm || 0, ss || 0, 0)`, y NaN es falsy, asi que el `|| 0`
// lo convertia en 0 y la funcion seguia como si nada. Entraba 18:25:00 y salia
// "06:25:05": compensaba los 5 s de traslado con exactitud de cronometro sobre una
// hora doce horas equivocada.
//
// Un `|| 0` sobre el resultado de un parseo no es un valor por defecto prudente: es
// el sitio exacto donde un dato roto se convierte en un dato limpio y falso. Por eso
// ahora se valida la forma ENTERA y se lanza. Que reviente es lo barato; lo caro es
// que un tecnico se vaya del poste creyendo que dejo la hora puesta.
const _HHMMSS = /^(\d{1,2}):(\d{2}):(\d{2})$/;

function _partirHora(horaStr) {
  if (typeof horaStr !== 'string') {
    throw new Error('Hora capturada inválida: no es texto');
  }
  const m = _HHMMSS.exec(horaStr.trim());
  if (!m) {
    throw new Error('Hora capturada inválida: "' + horaStr + '". Se espera HH:MM:SS ' +
                    'de 24 horas; un formato con "a. m."/"p. m." entraría 12 horas ' +
                    'equivocado sin que nada lo notara.');
  }
  const hh = Number(m[1]), mm = Number(m[2]), ss = Number(m[3]);
  if (hh > 23 || mm > 59 || ss > 59) {
    throw new Error('Hora capturada fuera de rango: "' + horaStr + '"');
  }
  return [hh, mm, ss];
}

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

    // Parsear hora capturada HH:MM:SS. Sin `|| 0` detras: ver la cabecera.
    const [hh, mm, ss] = _partirHora(snapshot.horaStr);
    const dateObj = new Date();
    dateObj.setHours(hh, mm, ss, 0);
    dateObj.setSeconds(dateObj.getSeconds() + elapsedSeg);

    // toTimeString() -no toLocaleTimeString()-: su formato lo fija la especificacion
    // y sale siempre en 24 h, sea cual sea el idioma del telefono.
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
