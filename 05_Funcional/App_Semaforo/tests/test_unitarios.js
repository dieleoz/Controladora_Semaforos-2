// ===== tests/test_unitarios.js =====
// Suite de Pruebas Unitarias TDD Automatizadas para App IOT-VIAL V9.0

const IOT_CONFIG = require('../js/config.js');
const NMEAParser = require('../js/nmea_parser.js');
const SiteManager = require('../js/site_manager.js');
const CourierRTC = require('../js/courier_rtc.js');

let passed = 0;
let failed = 0;

function assert(condition, message) {
  if (condition) {
    console.log(`  [OK] ${message}`);
    passed++;
  } else {
    console.error(`  [FAIL] ${message}`);
    failed++;
    process.exitCode = 1;
  }
}

console.log('='.repeat(80));
console.log(' 🧪 EJECUCIÓN DE TEST UNITARIOS TDD (COBERTURA TOTAL V9.0)');
console.log('='.repeat(80));

// 1. Checksum XOR y Formato NMEA
console.log('\n--- 1. Checksum XOR y Formato NMEA ---');
const crc1 = NMEAParser.calcularChecksum('STATUS,MODO:AUTO');
assert(typeof crc1 === 'string' && crc1.length === 2, 'Cálculo de Checksum XOR exacto (2 caracteres hex)');
const tramaFmt = NMEAParser.formatearTrama('STATUS,MODO:AUTO');
assert(tramaFmt.startsWith('$') && tramaFmt.includes('*') && tramaFmt.endsWith('\r\n'), 'Formato NMEA con $, *CRC y \\r\\n');

// 2. Parser $STATUS, $ALARM, $ERR
console.log('\n--- 2. Parseo de Telemetría NMEA ($STATUS, $ALARM, $ERR) ---');
// La trama es la que EMITE bluetooth.cpp:216, no una inventada. Se cambio en N-75:
// estas seis comprobaciones exigian FASE, TOT_FASE, BAT1/BAT2, RADIO y TELA, campos
// que ninguna punta manda, asi que daban PASS sobre un protocolo imaginario.
const rawStatus = '$STATUS,NODE:MAESTRO,SERIE:M-2026-A1B2,MODO:AUTO,ESTADO:V1_R2,T:38,RF:98%,RTT:82ms,BAT:12.6,HORA:18:25:00*00';
const parsed = NMEAParser.parseStatus(rawStatus.split('*')[0].substring(1));
assert(parsed && parsed.modo === 'AUTO', 'Parser lee MODO:AUTO');
assert(parsed && parsed.estado === 'V1_R2', 'Parser lee ESTADO:V1_R2 (el campo real, no FASE)');
assert(parsed && parsed.restante === 38, 'Parser lee T:38 como segundos restantes');
assert(parsed && parsed.bat === 12.6, 'Parser lee BAT:12.6 (una sola bateria, no BAT1/BAT2)');
assert(parsed && parsed.rf === 98, 'Parser lee RF:98% descartando el signo de porcentaje');
assert(parsed && parsed.hora === '18:25:00', 'Parser corta por el PRIMER : y no trunca HORA a 18');
assert(parsed && parsed.serie === 'M-2026-A1B2', 'Parser lee SERIE de silicio');

// Control negativo: una trama sin campos NO puede devolver un equipo sano. Antes
// devolvia modo AUTO, fase VERDE_P1 y 12.0 V por defecto.
const vacia = NMEAParser.parseStatus('STATUS');
assert(vacia && vacia.modo === undefined && vacia.bat === undefined,
  'Control negativo: una trama $STATUS sin campos no inventa modo ni bateria');

const parsedAlarm = NMEAParser.parseAlarm('ALARM,NODE:MAESTRO,EVENTO:RADIO_FAIL,CAUSA:Timeout RS485,ACCION:AMBAR');
assert(parsedAlarm && parsedAlarm.codigo === 'RADIO_FAIL', 'Parser detecta trama $ALARM por clave EVENTO');
assert(parsedAlarm && parsedAlarm.accion === 'AMBAR', 'Parser lee la ACCION que tomo el equipo');

const parsedErr = NMEAParser.parseError('ERR,SET_MODO,PIN_INCORRECTO');
assert(parsedErr && parsedErr.cmd === 'SET_MODO', 'Parser detecta trama $ERR');

// 3. Generación y Validación de Comandos con PIN
console.log('\n--- 3. Comandos con PIN y Validación de Rangos ---');
const cmdAuto = NMEAParser.generarComando('1234', 'SET_MODO', 'AUTO');
// includes() no puede ver un envoltorio: es cierto tambien de '$CMD:PIN:...*XX', que
// es una trama que el despachador del firmware RECHAZA. Estas tres lineas dieron
// verde durante meses sobre exactamente esa trama mala. Se compara la cadena ENTERA,
// terminador incluido, porque el terminador es parte del contrato.
assert(cmdAuto === 'CMD:PIN:1234:SET_MODO:AUTO\r\n', 'Generación de comando SET_MODO con PIN 1234');

const cmdRojo = NMEAParser.generarComando('1234', 'FORZAR_ROJO');
assert(cmdRojo === 'CMD:PIN:1234:FORZAR_ROJO\r\n', 'Generación de comando FORZAR_ROJO');

const cmdTiempos = NMEAParser.generarComando('1234', 'SET_TIEMPOS', '2,2,15');
assert(cmdTiempos === 'CMD:PIN:1234:SET_TIEMPOS:2,2,15\r\n', 'Generación de comando SET_TIEMPOS (Verde:2m, Rojo:2m, Despeje:15s)');

try {
  NMEAParser.generarComando('123', 'SET_MODO');
  assert(false, 'Debería rechazar PIN menor a 4 dígitos');
} catch (e) {
  assert(true, 'Rechazo seguro de PIN con longitud menor a 4 dígitos');
}

try {
  NMEAParser.generarComando('12AB', 'SET_MODO');
  assert(false, 'Debería rechazar PIN alfanumérico');
} catch (e) {
  assert(true, 'Rechazo seguro de PIN alfanumérico');
}

// 4. Validación de Tiempos de Ciclo (TDD)
console.log('\n--- 4. Validación de Rangos de Tiempos de Ciclo (TDD) ---');
function validarTiemposCiclo(verdeMin, rojoMin, despejeSeg) {
  if (verdeMin < 1 || verdeMin > 15) return { valid: false, error: 'Verde fuera de rango (1-15 min)' };
  if (rojoMin < 1 || rojoMin > 15) return { valid: false, error: 'Rojo fuera de rango (1-15 min)' };
  if (despejeSeg < 10 || despejeSeg > 90) return { valid: false, error: 'Despeje fuera de rango (10-90 seg)' };
  return { valid: true };
}

assert(validarTiemposCiclo(2, 2, 15).valid === true, 'Tiempos nominales (2m, 2m, 15s) válidos');
assert(validarTiemposCiclo(0, 2, 15).valid === false, 'Rechaza Verde < 1 min');
assert(validarTiemposCiclo(20, 2, 15).valid === false, 'Rechaza Verde > 15 min');
assert(validarTiemposCiclo(2, 2, 5).valid === false, 'Rechaza Despeje < 10 seg (Riesgo de colisión)');
assert(validarTiemposCiclo(2, 2, 120).valid === false, 'Rechaza Despeje > 90 seg');

// 5. Asistente Courier RTC
console.log('\n--- 5. Asistente Courier RTC ---');
const snap = CourierRTC.capturarMaestro('18:25:00', 'VERDE_P1', 38);
assert(snap && snap.horaStr === '18:25:00', 'Captura de hora y fase del Maestro');

const comp = CourierRTC.calcularCompensacion(snap, snap.timestampCaptura + 120000);
assert(comp.elapsedSeg === 120, 'Cálculo exacto de 120 segundos de traslado');
assert(comp.horaCompensada === '18:27:00', 'Compensación horaria exacta (18:25:00 + 2m = 18:27:00)');

// 6. Gestor de Cruces Viales (CRUD LocalStorage)
console.log('\n--- 6. Gestor de Cruces Viales (CRUD LocalStorage) ---');
global.localStorage = {
  store: {},
  getItem(k) { return this.store[k] || null; },
  setItem(k, v) { this.store[k] = String(v); },
  removeItem(k) { delete this.store[k]; }
};

const cruces = SiteManager.obtenerCruces();
assert(Array.isArray(cruces) && cruces.length >= 2, 'Carga de cruces viales por defecto');

const nuevoCruce = SiteManager.agregarCruce('Obra Túnel 5', 'PR 50+000');
assert(nuevoCruce && nuevoCruce.nombre === 'Obra Túnel 5', 'Creación de nuevo cruce vial');

const filtro = SiteManager.filtrarCruces('Túnel');
assert(filtro.length >= 1, 'Búsqueda y filtrado en tiempo real de cruces');

const eliminado = SiteManager.eliminarCruce(nuevoCruce.id);
assert(eliminado === true, 'Eliminación correcta de cruce vial');

// 7. Roles Operario vs Técnico (TDD)
console.log('\n--- 7. Lógica de Roles y Seguridad de Acceso (TDD) ---');
function verificarAccesoAdmin(pinIngresado, pinCorrecto) {
  return pinIngresado === pinCorrecto;
}
assert(verificarAccesoAdmin('1234', '1234') === true, 'Autenticación exitosa con PIN 1234');
assert(verificarAccesoAdmin('0000', '1234') === false, 'Rechazo de PIN erróneo');

console.log('\n' + '='.repeat(80));
console.log(` RESUMEN TDD: ${passed} PASS | ${failed} FALLAS  (Total: ${passed + failed})`);
console.log('='.repeat(80));
if (failed === 0) {
  console.log('🎉 TODAS LAS PRUEBAS UNITARIAS TDD PASARON AL 100%\n');
}
