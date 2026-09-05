// ===== tests/test_unitarios.js =====
// Suite de Pruebas Unitarias TDD Automatizadas para App IOT-VIAL V9.0

const IOT_CONFIG = require('../js/config.js');
const NMEAParser = require('../js/nmea_parser.js');
const SiteManager = require('../js/site_manager.js');
const CourierRTC = require('../js/courier_rtc.js');
const RegistroCrudo = require('../js/depuracion.js');

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

// [INVERTIDA el 05/09, CLAUDE.md 8.quater] Esta linea EXIGIA el defecto.
//
// Le daba de comer 'ERR,SET_MODO,PIN_INCORRECTO' -campos por POSICION, sin claves- y
// ninguna punta emite eso. Las dos bluetooth.cpp y el despachador del ESP32 mandan
// 'ERR,CMD:SET_TIEMPOS,DESC:RANGO'. Sobre la trama de verdad el parser viejo devolvia
// cmd = 'CMD:SET_TIEMPOS' -el nombre del campo pegado al valor- y esta prueba seguia en
// verde, porque medía el parser contra un protocolo inventado a su medida.
//
// Ahora se le da la trama REAL y se exige el valor limpio. La prueba no se borra ni se
// reescribe entera: lo que afirmaba -"el parser distingue una trama $ERR"- sigue
// valiendo; lo que cambia es contra qué.
const parsedErr = NMEAParser.parseError('ERR,CMD:SET_TIEMPOS,DESC:RANGO');
assert(parsedErr && parsedErr.cmd === 'SET_TIEMPOS',
  'Parser lee el CMD de un $ERR real por su CLAVE, no por posicion (cmd=CMD:... era el defecto)');
assert(parsedErr && parsedErr.desc === 'RANGO',
  'y lee el DESC igual: es el literal con el que se busca el motivo en el roadmap');

// Y EL CONTROL QUE LE FALTABA A LA INVERSION. Un parser que devolviera siempre
// undefined pasaria las dos lineas de arriba si se hubieran escrito al reves; y uno que
// se inventara un 'UNKNOWN' -que es lo que hacia el viejo con el CMD ausente- diria que
// sabe algo que no sabe. Sobre una trama $ERR SIN claves no hay nada que leer, y eso es
// lo que tiene que devolver.
const errSinClaves = NMEAParser.parseError('ERR,SET_MODO,PIN_INCORRECTO');
assert(errSinClaves && errSinClaves.cmd === undefined,
  'Control negativo: ante un $ERR sin claves el parser no inventa un CMD ("UNKNOWN" era relleno)');

// EL PARSER QUE SE PRUEBA ES EL QUE CORRE EN EL TELEFONO (05/09).
//
// Hasta hoy no lo era: la app partia las tramas con su propio _camposNmea() y este
// fichero probaba parseStatus(), que es otro codigo. Ahora los dos entran por
// camposDeTrama(), asi que estas cuatro lineas ejercen la funcion que de verdad viaja
// en la APK. El pack app_12_un_solo_parser vigila que siga siendo asi.
const campos = NMEAParser.camposDeTrama('$STATUS,HORA:18:25:00,T:--,ESC:?'.split(','));
assert(campos.HORA === '18:25:00',
  'camposDeTrama corta por el PRIMER : y no trunca HORA a 18 (N-62)');
assert(campos.T === '--',
  'camposDeTrama devuelve la MARCA de ausencia tal cual, sin convertirla en 0');
assert(campos.ESC === '?',
  'camposDeTrama no traduce: "?" es el Maestro diciendo que no sabe el color del Esclavo');
assert(Object.keys(campos).length === 3,
  'camposDeTrama no inventa campos: solo devuelve los que traia la trama');

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

// 8. Validación de checksum: la función que llevaba meses sin llamador
console.log('\n--- 8. Validación de Checksum NMEA (B1: conectada en la V2) ---');
// El XOR es el MISMO del firmware -Maestro/src/bluetooth.cpp, enviarTramaConCrc()
// calcula sobre `payload + 1`, o sea saltando el '$'-. Aqui se comprueba contra un
// numero calculado a mano, no contra la propia funcion: comparar calcularChecksum()
// consigo misma daria PASS con el XOR roto.
const tramaBuena = '$STATUS,NODE:ESCLAVO,SERIE:SEM-E-01,MODO:AUTO,ESTADO:R1_V2,T:28,RF:95,RTT:75,BAT:12.8,HORA:14:30:00*04';
const vBuena = NMEAParser.validarTrama(tramaBuena);
assert(vBuena.valida === true, 'validarTrama() acepta una trama con el checksum correcto');
assert(vBuena.payload.startsWith('STATUS,'),
  'y devuelve el payload SIN el $, que es sobre lo que el firmware calcula el XOR');

// La misma trama con el checksum que llevaba escrito a mano el arnes de DOM. La app la
// PINTABA: parseNmeaTelemetry() hacia line.split('*')[0] y tiraba el CRC sin leerlo.
const vMala = NMEAParser.validarTrama(tramaBuena.replace('*04', '*5F'));
assert(vMala.valida === false && vMala.motivo === 'CHECKSUM',
  'Rechaza la misma trama con el checksum cambiado, y lo clasifica como CHECKSUM');
assert(vMala.esperado === '04' && vMala.recibido === '5F',
  `El rechazo dice los dos números, que es lo que se mira al diagnosticar: esperado ${vMala.esperado}, recibido ${vMala.recibido}`);

const vSinAst = NMEAParser.validarTrama('$STATUS,NODE:ESCLAVO');
assert(vSinAst.valida === false && vSinAst.motivo === 'SIN_FORMA',
  'Una línea sin el * se rechaza como SIN_FORMA, no como checksum malo');
const vSinDolar = NMEAParser.validarTrama('STATUS,NODE:ESCLAVO*04');
assert(vSinDolar.valida === false && vSinDolar.motivo === 'SIN_FORMA',
  'Y una sin el $ tampoco pasa: el $ es parte del contrato del cable');

// El motivo va en clave y no en la frase: agrupar rechazos por un texto en castellano
// se rompe el dia que alguien corrija una tilde.
assert(typeof vMala.motivo === 'string' && vMala.motivo === vMala.motivo.toUpperCase(),
  'El motivo es una clave en mayúsculas, contable; la frase para el técnico va aparte');

// 9. La cinta de tramas en crudo (A1)
console.log('\n--- 9. Cinta de tramas en crudo (modo depuración) ---');
RegistroCrudo.limpiar();
assert(RegistroCrudo.todas().length === 0 && RegistroCrudo.descartados === 0,
  'La cinta arranca vacía y sin descartes');

const t0 = 1700000000000;
RegistroCrudo.anotar('$STATUS,NODE:ESCLAVO,T:28*XX\r\n', { aceptada: true, tipo: '$STATUS' }, t0);
RegistroCrudo.anotar('$STATUS,ROTA*00\r\n',
  { aceptada: false, motivo: 'CHECKSUM', detalle: 'esperado 16, recibido 00', tipo: '$STATUS' }, t0 + 1000);
const c9 = RegistroCrudo.contadores(t0 + 2000);
assert(c9.total === 2 && c9.aceptadas === 1 && c9.rechazadas === 1,
  `Los contadores separan aceptadas de rechazadas: ${c9.aceptadas}/${c9.rechazadas}`);
assert(c9.porMotivo.CHECKSUM === 1, 'y desglosan por motivo');

// La ventana no es decoración: "12 rechazadas" sin un "de cuándo" no dice nada.
const cViejo = RegistroCrudo.contadores(t0 + RegistroCrudo.VENTANA_MS + 5000);
assert(cViejo.total === 0 && cViejo.fueraDeVentana === 2,
  'Las tramas más viejas que la ventana salen del recuento Y se declaran como fuera, no desaparecen');

// El escapado toca la REPRESENTACION, nunca el dato: un CR suelto en mitad de la lista
// ensenaria una trama distinta de la que entro.
assert(RegistroCrudo.escapar('A\r\nB\tC') === 'A\\r\\nB\\tC',
  'Los caracteres de control salen escapados al pintar y al exportar');
assert(RegistroCrudo.escapar(String.fromCharCode(7)) === '\\x07',
  'y los no imprimibles salen en hexadecimal, no se comen en silencio');
assert(RegistroCrudo.todas()[0].linea.includes('\r\n'),
  'pero la línea guardada conserva el terminador tal y como llegó');

// Tope duro y recorte CONTADO: un registro recortado en silencio se lee como completo.
RegistroCrudo.limpiar();
for (let i = 0; i < RegistroCrudo.TOPE + 25; i++) {
  RegistroCrudo.anotar('$STATUS,N:' + i + '*00', { aceptada: true, tipo: '$STATUS' }, t0 + i);
}
assert(RegistroCrudo.todas().length === RegistroCrudo.TOPE,
  `La cinta no crece sin límite: se queda en el tope de ${RegistroCrudo.TOPE}`);
assert(RegistroCrudo.descartados === 25,
  `y CUENTA las ${RegistroCrudo.descartados} que tiró: un recorte silencioso se lee como registro completo`);
assert(RegistroCrudo.aTexto(t0).includes('RECORTADO'),
  'El texto exportado lo dice también, que es donde acaba mirando quien no estuvo delante');

// Una línea absurdamente larga se corta y se dice cuánto llegó.
RegistroCrudo.limpiar();
const larga = '$' + 'A'.repeat(RegistroCrudo.LARGO_MAX + 50);
const regLargo = RegistroCrudo.anotar(larga, { aceptada: false, motivo: 'SIN_FORMA' }, t0);
assert(regLargo.cortada === true && regLargo.largoOriginal === larga.length,
  `Una línea de ${larga.length} caracteres se corta y se guarda su largo real`);
assert(RegistroCrudo.aTexto(t0).includes('CORTADA'),
  'y el export lo declara en vez de enseñar un trozo como si fuera la trama entera');

// El export lleva lo que hace falta para diagnosticar, y no necesita internet.
RegistroCrudo.limpiar();
RegistroCrudo.anotar('$STATUS,NODE:MAESTRO,T:7*00',
  { aceptada: false, motivo: 'CHECKSUM', detalle: 'esperado 3B, recibido 00' }, t0);
const txt = RegistroCrudo.aTexto(t0, { Cruce: 'KM 12' });
assert(txt.includes('$STATUS,NODE:MAESTRO,T:7*00') && txt.includes('CHECKSUM') &&
       txt.includes('esperado 3B') && txt.includes('KM 12'),
  'El export lleva la trama en crudo, el motivo, el detalle y el cruce');
assert(!/https?:\/\//.test(txt),
  'y no contiene ninguna URL: se compone en el teléfono porque en el cruce puede no haber internet');

// Una cinta vacía DECLARA que está vacía. Nunca una trama de ejemplo.
RegistroCrudo.limpiar();
const txtVacio = RegistroCrudo.aTexto(t0);
assert(txtVacio.includes('vacia') && !txtVacio.includes('$STATUS'),
  'Una cinta vacía se exporta diciendo que está vacía, sin inventar ninguna trama de ejemplo');

// Los motivos declarados tienen que tener descripción: un motivo que sale en pantalla
// como una clave a secas no le dice nada a quien está subido a la escalera.
assert(Object.keys(RegistroCrudo.MOTIVOS).length >= 3 &&
       Object.keys(RegistroCrudo.MOTIVOS).every(k => RegistroCrudo.MOTIVOS[k].length > 10),
  `Los ${Object.keys(RegistroCrudo.MOTIVOS).length} motivos de rechazo tienen texto que explica qué pasó`);

console.log('\n' + '='.repeat(80));
console.log(` RESUMEN TDD: ${passed} PASS | ${failed} FALLAS  (Total: ${passed + failed})`);
console.log('='.repeat(80));
if (failed === 0) {
  console.log('🎉 TODAS LAS PRUEBAS UNITARIAS TDD PASARON AL 100%\n');
}
