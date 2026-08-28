// ===== js/config.js =====
// Constantes del Protocolo de Telemetría y Límites Operativos IOT-VIAL V8.9 / V9.0

const IOT_CONFIG = {
  VERSION: '8.9.0',
  DEFAULT_PIN: '1234',
  BAUDRATE: 9600,
  
  // Rangos de Tiempos Permitidos por Firmware
  LIMITES_TIEMPO: {
    VERDE_MIN_MIN: 1,
    VERDE_MAX_MIN: 15,
    ROJO_MIN_MIN: 1,
    ROJO_MAX_MIN: 15,
    DESPEJE_MIN_SEG: 10,
    DESPEJE_MAX_SEG: 90
  },

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
