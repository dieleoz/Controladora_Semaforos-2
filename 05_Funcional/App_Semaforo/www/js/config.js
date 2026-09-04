// ===== js/config.js =====
// Constantes del Protocolo de Telemetría y Límites Operativos IOT-VIAL V8.9 / V9.0

const IOT_CONFIG = {
  VERSION: '8.9.0',
  DEFAULT_PIN: '1234',
  BAUDRATE: 9600,
  
  // LOS LIMITES DE TIEMPO SE RETIRAN DE AQUI (04/09/2026). NO SE ACTUALIZAN: SE BORRAN.
  //
  // Habia un LIMITES_TIEMPO con VERDE_MIN_MIN: 1 y ROJO_MIN_MIN: 1, bajo el rotulo
  // "Rangos de Tiempos Permitidos por Firmware". El firmware dice 3 desde el 04/09
  // -Maestro/src/modo_automatico.cpp-, asi que era una cifra CADUCADA con una frase
  // encima que la presentaba como medida. Es lo que CLAUDE.md §3.bis llama la version
  // silenciosa de la prueba muerta, y aqui con agravante: no la lee NADIE.
  //
  // Censado antes de tocarla: grep de LIMITES_TIEMPO y de IOT_CONFIG sobre toda la app
  // da CERO consumidores fuera de este fichero. La validacion real vive en app.js
  // -enRango(verde, 3, 15)- y en los min/max de index.html, y el pack
  // app_11_rangos_de_tiempos cruza esos dos contra el C++ en cada corrida.
  //
  // Por que se BORRA y no se corrige a 3: actualizarla habria creado una CUARTA copia
  // a mano de los mismos seis numeros -R-9 de contrato.h, que este repositorio ya ha
  // pagado tres veces- y ademas una copia que nadie usa, o sea otra que sincronizar
  // sin nada que la obligue. Un numero que nadie lee no protege; solo espera a que
  // alguien se lo crea. El pack vigila desde hoy que no vuelva a aparecer aqui.
  //
  // AVISO APARTE, y no se arregla en este commit: el resto de IOT_CONFIG tampoco tiene
  // consumidores. Retirarlo entero toca index.html, que en este momento esta abierto
  // por otro trabajo; queda anotado para no perderlo.

  // Modos de Operación
  MODOS: {
    AUTO: 'AUTO',
    MANUAL: 'MANUAL',
    AMBAR: 'AMBAR',
    DEGRADADO: 'DEGRADADO',
    APAGADO: 'APAGADO'
  },

  // UUIDs BLE para Módulos Bluetooth Low Energy (ej. HM-10 / JDY-33)
  BLE: {
    SERVICE_UUID: '0000ffe0-0000-1000-8000-00805f9b34fb',
    CHARACTERISTIC_UUID: '0000ffe1-0000-1000-8000-00805f9b34fb'
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = IOT_CONFIG;
}
