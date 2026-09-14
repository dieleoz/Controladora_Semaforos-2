// =============================================================================
// test_unitarios_app.js — Suite de Pruebas Unitarias del Ecosistema App IOT-VIAL
// =============================================================================
// Cobertura:
// 1. Cálculo y Verificación de Checksum NMEA 0183
// 2. Parser de Telemetría ($STATUS, $ALARM, $EVENT, $ACK, $ERR)
// 3. Robustez y Fuzzing de Tramas Corruptas
// 4. Algoritmo y Compensación Temporal Courier RTC
// 5. Gestor de Cruces Viales (CRUD LocalStorage)
// 6. Validadores de Rangos y Parámetros Viales (SET_TIEMPOS)
// 7. Generador de Comandos Seguros con Barrera de PIN
// =============================================================================

const assert = require('assert');

// 🔴 ESTE FICHERO ERA LA TERCERA COPIA DEL PARSER, Y LA PEOR DE LAS TRES (05/09).
//
// La cabecera de arriba dice "Modulos Puros de la App", y esa frase era el problema: lo
// que habia debajo NO eran los modulos de la app, eran reimplementaciones a mano del
// checksum, del parser y del generador de comandos, escritas en este mismo fichero y
// probadas contra si mismas. 29 comprobaciones en verde que no tocaban una sola linea de
// lo que se instala en el telefono.
//
// La forma es la de CLAUDE.md 3.bis -"una segunda copia del firmware escrita a mano que
// alguien sincroniza"- con la vuelta de tuerca de que aqui la copia es la que tiene las
// pruebas. Y ya habia derivado: el bucle de abajo partia los campos con
// `tokens[i].split(':')` mirando `pair.length`, que es OTRO criterio que el primer ':'
// de N-62. Acertaba por casualidad en HORA y nadie lo habia comprobado nunca.
//
// Desde hoy el checksum y el parser SON los de js/nmea_parser.js. Lo que este fichero
// conserva propio es solo la FORMA de su resultado -{type, ...} y {error:'BAD_CHECKSUM'}-,
// porque sus 29 assert la usan y cambiarla seria reescribir la suite entera para
// arreglar el parser: dos cosas distintas en un commit.
const NMEAParser = require('./js/nmea_parser.js');

let totalTests = 0;
let passedTests = 0;
let failedTests = 0;

function runTest(suiteName, testName, testFn) {
  totalTests++;
  try {
    testFn();
    console.log(`  [OK] [${suiteName}] ${testName}`);
    passedTests++;
  } catch (err) {
    console.error(`  [FAIL] [${suiteName}] ${testName}: ${err.message}`);
    failedTests++;
  }
}

// -----------------------------------------------------------------------------
// IMPLEMENTACIÓN DE FUNCIONES A TESTEAR (Módulos Puros de la App)
// -----------------------------------------------------------------------------

// El XOR ya no se reimplementa: se llama al de la app. Era byte a byte el mismo codigo,
// y "el mismo codigo" mantenido en dos sitios es el que deja de serlo sin avisar.
function calcularChecksumNmea(cadena) {
  return NMEAParser.calcularChecksum(cadena);
}

function formatearTramaNmea(payload) {
  const crc = calcularChecksumNmea(payload);
  return `${payload}*${crc}\r\n`;
}

function parseNmeaTelemetry(line) {
  if (!line || typeof line !== 'string') return null;
  const clean = line.trim();
  if (!clean.startsWith('$') || !clean.includes('*')) return null;

  const parts = clean.split('*');
  const payloadWithDollar = parts[0];
  const checksumReceived = parts[1];

  const payloadSinDollar = payloadWithDollar.substring(1);
  const expectedCrc = calcularChecksumNmea(payloadSinDollar);

  if (checksumReceived.toUpperCase() !== expectedCrc.toUpperCase()) {
    return { error: 'BAD_CHECKSUM', expected: expectedCrc, received: checksumReceived };
  }

  // EL BUCLE PROPIO SE RETIRA. Partia con split(':') y decidia por `pair.length`, que
  // es un criterio distinto del que corre en el telefono -el PRIMER ':' de N-62-. Daba
  // el mismo resultado en las tramas de esta suite y nadie habia comprobado que lo diera
  // en las demas: un valor con TRES ':' o uno que empiece por ':' se leen distinto en
  // los dos. Ahora se parte con la funcion de la app, que es la que hay que probar.
  const tokens = payloadSinDollar.split(',');
  return Object.assign({ type: tokens[0] }, NMEAParser.camposDeTrama(tokens));
}

function buildCommand(pin, cmdName, params = '') {
  if (!pin || pin.length !== 4 || !/^\d{4}$/.test(pin)) {
    throw new Error('PIN inválido: debe contener exactamente 4 dígitos numéricos.');
  }
  let payload = `CMD:PIN:${pin}:${cmdName}`;
  if (params) {
    payload += `:${params}`;
  }
  return payload + '\r\n';
}

function validateTiempos(verdeMin, rojoMin, despejeSeg) {
  const v = parseInt(verdeMin, 10);
  const r = parseInt(rojoMin, 10);
  const d = parseInt(despejeSeg, 10);

  if (isNaN(v) || v < 3 || v > 15) {
    return { valid: false, error: 'VERDE_OUT_OF_RANGE', msg: 'Verde debe estar entre 3 y 15 min.' };
  }
  if (isNaN(r) || r < 3 || r > 15) {
    return { valid: false, error: 'ROJO_OUT_OF_RANGE', msg: 'Rojo debe estar entre 3 y 15 min.' };
  }
  if (isNaN(d) || d < 10 || d > 90) {
    return { valid: false, error: 'DESPEJE_OUT_OF_RANGE', msg: 'Despeje todo-rojo debe estar entre 10 y 90 seg.' };
  }

  return { valid: true, verde: v, rojo: r, despeje: d };
}

function calculateCourierCompensation(tCapturaMs, tInyeccionMs) {
  if (tInyeccionMs < tCapturaMs) {
    throw new Error('Inconsistencia temporal: tiempo de inyección anterior a captura');
  }
  const deltaMs = tInyeccionMs - tCapturaMs;
  const fechaInyectada = new Date(tCapturaMs + deltaMs);

  const y = fechaInyectada.getFullYear();
  const mo = String(fechaInyectada.getMonth() + 1).padStart(2, '0');
  const d = String(fechaInyectada.getDate()).padStart(2, '0');
  const h = String(fechaInyectada.getHours()).padStart(2, '0');
  const mi = String(fechaInyectada.getMinutes()).padStart(2, '0');
  const s = String(fechaInyectada.getSeconds()).padStart(2, '0');

  return {
    deltaMs,
    deltaSeconds: Math.floor(deltaMs / 1000),
    cmdRtc: `SET_RTC:${y}-${mo}-${d},${h}:${mi}:${s}`,
    isoDate: fechaInyectada.toISOString()
  };
}

class InMemorySiteManager {
  static _seq = 0;

  constructor(initialSites = []) {
    this.sites = initialSites.length ? [...initialSites] : [
      { id: 'site-1', name: 'Cruce Km 12 · El Sisga', location: 'PR 12+400', p1: '👑 Maestro (P1)', p2: '📡 Esclavo (P2)' }
    ];
  }

  getAll(filter = '') {
    const q = filter.trim().toLowerCase();
    return this.sites.filter(s =>
      s.name.toLowerCase().includes(q) ||
      (s.location && s.location.toLowerCase().includes(q))
    );
  }

  getById(id) {
    return this.sites.find(s => s.id === id) || null;
  }

  save(siteData) {
    if (!siteData.name || !siteData.name.trim()) {
      throw new Error('El nombre del cruce es obligatorio.');
    }
    if (siteData.id) {
      const idx = this.sites.findIndex(s => s.id === siteData.id);
      if (idx >= 0) {
        this.sites[idx] = { ...this.sites[idx], ...siteData };
        return this.sites[idx];
      }
    }
    const newSite = {
      // N-75: con Math.random() sobre 1000 valores, 20 altas colisionan una de cada
      // seis corridas y la suite fallaba de forma intermitente. Un instrumento que da
      // resultados distintos con el mismo codigo no mide: ensena a re-correr hasta que
      // salga verde. El doble replica ahora el contador del modulo real.
      id: 'site-' + Date.now() + '-' + (++InMemorySiteManager._seq),
      name: siteData.name.trim(),
      location: (siteData.location || '').trim(),
      p1: (siteData.p1 || '👑 Maestro (P1)').trim(),
      p2: (siteData.p2 || '📡 Esclavo (P2)').trim()
    };
    this.sites.unshift(newSite);
    return newSite;
  }

  delete(id) {
    if (this.sites.length <= 1) {
      throw new Error('Invariante: no se puede eliminar el único cruce existente.');
    }
    const idx = this.sites.findIndex(s => s.id === id);
    if (idx >= 0) {
      const deleted = this.sites.splice(idx, 1)[0];
      return deleted;
    }
    return null;
  }
}

// =============================================================================
// EJECUCIÓN DE LAS PRUEBAS UNITARIAS
// =============================================================================

console.log('='.repeat(80));
console.log(' 🧪 EJECUCIÓN DE TEST UNITARIOS — APP IOT-VIAL V8.9');
console.log('='.repeat(80));

// --- SUITE 1: Protocolo NMEA & Checksums ---
runTest('NMEA', 'Cálculo de Checksum XOR exacto', () => {
  const payload = 'STATUS,NODE:MAESTRO,BAT:12.6';
  const crc = calcularChecksumNmea(payload);
  assert.strictEqual(typeof crc, 'string');
  assert.strictEqual(crc.length, 2);
  // Validar formato hexadecimal
  assert.match(crc, /^[0-9A-F]{2}$/);
});

runTest('NMEA', 'Formateo de Trama con prefijo $ y sufijo *CRC\\r\\n', () => {
  const trama = formatearTramaNmea('$STATUS,NODE:MAESTRO');
  assert.ok(trama.startsWith('$STATUS,NODE:MAESTRO*'));
  assert.ok(trama.endsWith('\r\n'));
});

runTest('NMEA', 'Parser de Telemetría $STATUS completa', () => {
  const raw = '$STATUS,NODE:MAESTRO,ID:SEM-M-01,SITE:Km 12 Sisga,PAIR:SEM-E-01,MODO:AUTO,ESTADO:V1_R2,T:35,RF:98,RTT:82,BAT:12.6,HORA:18:25:00';
  const crc = calcularChecksumNmea(raw.substring(1));
  const full = `${raw}*${crc}\r\n`;

  const parsed = parseNmeaTelemetry(full);
  assert.ok(parsed !== null);
  assert.strictEqual(parsed.type, 'STATUS');
  assert.strictEqual(parsed.NODE, 'MAESTRO');
  assert.strictEqual(parsed.ID, 'SEM-M-01');
  assert.strictEqual(parsed.MODO, 'AUTO');
  assert.strictEqual(parsed.ESTADO, 'V1_R2');
  assert.strictEqual(parsed.T, '35');
  assert.strictEqual(parsed.RF, '98');
  assert.strictEqual(parsed.RTT, '82');
  assert.strictEqual(parsed.BAT, '12.6');
  assert.strictEqual(parsed.HORA, '18:25:00');
});

runTest('NMEA', 'Parser de Trama de Alarma $ALARM', () => {
  const raw = '$ALARM,NODE:ESCLAVO,EVENTO:ENLACE_PERDIDO,CAUSA:TIMEOUT_RADIO_12S,ACCION:MODO_AMBAR_FAILSAFE,HORA:19:10:00';
  const crc = calcularChecksumNmea(raw.substring(1));
  const full = `${raw}*${crc}\r\n`;

  const parsed = parseNmeaTelemetry(full);
  assert.strictEqual(parsed.type, 'ALARM');
  assert.strictEqual(parsed.EVENTO, 'ENLACE_PERDIDO');
  assert.strictEqual(parsed.ACCION, 'MODO_AMBAR_FAILSAFE');
});

runTest('NMEA', 'Parser de Trama de Error $ERR', () => {
  const raw = '$ERR,CMD:SET_TIEMPOS,DESC:RANGO';
  const crc = calcularChecksumNmea(raw.substring(1));
  const full = `${raw}*${crc}\r\n`;

  const parsed = parseNmeaTelemetry(full);
  assert.strictEqual(parsed.type, 'ERR');
  assert.strictEqual(parsed.CMD, 'SET_TIEMPOS');
  assert.strictEqual(parsed.DESC, 'RANGO');
});

runTest('NMEA', 'Detección de Trama Corrupta (Bad Checksum)', () => {
  const bad = '$STATUS,NODE:MAESTRO,BAT:12.6*FF\r\n';
  const parsed = parseNmeaTelemetry(bad);
  assert.ok(parsed && parsed.error === 'BAD_CHECKSUM');
});

runTest('NMEA', 'Descarte de tramas vacías o sin formato', () => {
  assert.strictEqual(parseNmeaTelemetry(''), null);
  assert.strictEqual(parseNmeaTelemetry('HOLA_MUNDO'), null);
  assert.strictEqual(parseNmeaTelemetry(null), null);
  assert.strictEqual(parseNmeaTelemetry(undefined), null);
});

// --- SUITE 2: Generador de Comandos y Barrera PIN ---
runTest('Comandos', 'Generación de comando SET_MODO:AUTO con PIN 1234', () => {
  const cmd = buildCommand('1234', 'SET_MODO', 'AUTO');
  assert.strictEqual(cmd, 'CMD:PIN:1234:SET_MODO:AUTO\r\n');
});

runTest('Comandos', 'Generación de comando FORZAR_ROJO de emergencia', () => {
  const cmd = buildCommand('1234', 'FORZAR_ROJO');
  assert.strictEqual(cmd, 'CMD:PIN:1234:FORZAR_ROJO\r\n');
});

runTest('Comandos', 'Generación de comando SOLICITAR_PASO desde Esclavo', () => {
  const cmd = buildCommand('1234', 'SOLICITAR_PASO');
  assert.strictEqual(cmd, 'CMD:PIN:1234:SOLICITAR_PASO\r\n');
});

runTest('Comandos', 'Generación de comando SET_TIEMPOS:5,6,30', () => {
  const cmd = buildCommand('1234', 'SET_TIEMPOS', '5,6,30');
  assert.strictEqual(cmd, 'CMD:PIN:1234:SET_TIEMPOS:5,6,30\r\n');
});

runTest('Comandos', 'Rechazo de PIN con longitud menor a 4 dígitos', () => {
  assert.throws(() => buildCommand('12', 'FORZAR_ROJO'), /PIN inválido/);
});

runTest('Comandos', 'Rechazo de PIN con caracteres alfanuméricos', () => {
  assert.throws(() => buildCommand('12AB', 'FORZAR_ROJO'), /PIN inválido/);
});

// --- SUITE 3: Validación de Parámetros Viales (SET_TIEMPOS) ---
runTest('Tiempos', 'Parámetros válidos en rango estándar (Verde=3m, Rojo=4m, Despeje=25s)', () => {
  const res = validateTiempos(3, 4, 25);
  assert.strictEqual(res.valid, true);
  assert.strictEqual(res.verde, 3);
  assert.strictEqual(res.rojo, 4);
  assert.strictEqual(res.despeje, 25);
});

runTest('Tiempos', 'Rechazo de tiempo Verde menor a 3 minutos (2 min)', () => {
  const res = validateTiempos(2, 4, 25);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'VERDE_OUT_OF_RANGE');
});

runTest('Tiempos', 'Rechazo de tiempo Verde mayor a 15 minutos (20 min)', () => {
  const res = validateTiempos(20, 4, 25);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'VERDE_OUT_OF_RANGE');
});

runTest('Tiempos', 'Rechazo de tiempo Rojo menor a 3 minutos (1 min)', () => {
  const res = validateTiempos(3, 1, 25);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'ROJO_OUT_OF_RANGE');
});

runTest('Tiempos', 'Rechazo de tiempo Rojo mayor a 15 minutos', () => {
  const res = validateTiempos(3, 16, 25);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'ROJO_OUT_OF_RANGE');
});

runTest('Tiempos', 'Rechazo de Despeje Todo-Rojo menor a 10 segundos (5 seg)', () => {
  const res = validateTiempos(3, 4, 5);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'DESPEJE_OUT_OF_RANGE');
});

runTest('Tiempos', 'Rechazo de Despeje Todo-Rojo mayor a 90 segundos (120 seg)', () => {
  const res = validateTiempos(3, 4, 120);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'DESPEJE_OUT_OF_RANGE');
});

// --- SUITE 4: Asistente Courier RTC ---
runTest('Courier RTC', 'Compensación exacta con viaje de 3 minutos y 45 segundos (225.000 ms)', () => {
  const t0 = new Date('2026-08-27T10:00:00.000Z').getTime();
  const t1 = t0 + (3 * 60 + 45) * 1000;

  const result = calculateCourierCompensation(t0, t1);
  assert.strictEqual(result.deltaSeconds, 225);
  assert.ok(result.cmdRtc.startsWith('SET_RTC:2026-08-27,'));
});

runTest('Courier RTC', 'Inconsistencia temporal rechazada si t1 < t0', () => {
  const t0 = 10000;
  const t1 = 5000;
  assert.throws(() => calculateCourierCompensation(t0, t1), /Inconsistencia temporal/);
});

// --- SUITE 5: Gestor de Cruces (CRUD LocalStorage) ---
runTest('Gestor Cruces', 'Inicialización con cruces por defecto', () => {
  const mgr = new InMemorySiteManager();
  const all = mgr.getAll();
  assert.strictEqual(all.length, 1);
  assert.strictEqual(all[0].name, 'Cruce Km 12 · El Sisga');
});

runTest('Gestor Cruces', 'Creación de nuevo cruce vial (➕)', () => {
  const mgr = new InMemorySiteManager();
  const nuevo = mgr.save({
    name: 'Obra Variante Km 45',
    location: 'PR 45+200',
    p1: '👑 Maestro (Túnel)',
    p2: '📡 Esclavo (Salida)'
  });
  assert.ok(nuevo.id.startsWith('site-'));
  assert.strictEqual(mgr.getAll().length, 2);
  assert.strictEqual(mgr.getAll()[0].name, 'Obra Variante Km 45');
});

runTest('Gestor Cruces', 'Edición de cruce existente (✏️)', () => {
  const mgr = new InMemorySiteManager();
  const all = mgr.getAll();
  const targetId = all[0].id;

  mgr.save({
    id: targetId,
    name: 'Cruce Km 12 · El Sisga (Actualizado)',
    location: 'PR 12+500'
  });

  const updated = mgr.getById(targetId);
  assert.strictEqual(updated.name, 'Cruce Km 12 · El Sisga (Actualizado)');
  assert.strictEqual(updated.location, 'PR 12+500');
});

runTest('Gestor Cruces', 'Filtrado en tiempo real por búsqueda', () => {
  const mgr = new InMemorySiteManager([
    { id: '1', name: 'Túnel Oriente', location: 'Km 5' },
    { id: '2', name: 'Variante Norte', location: 'Km 18' },
    { id: '3', name: 'Puente Río Bogotá', location: 'Km 32' }
  ]);

  const f1 = mgr.getAll('túnel');
  assert.strictEqual(f1.length, 1);
  assert.strictEqual(f1[0].name, 'Túnel Oriente');

  const f2 = mgr.getAll('Km 18');
  assert.strictEqual(f2.length, 1);
  assert.strictEqual(f2[0].name, 'Variante Norte');

  const f3 = mgr.getAll('inexistente');
  assert.strictEqual(f3.length, 0);
});

runTest('Gestor Cruces', 'Eliminación de cruce vial (🗑️)', () => {
  const mgr = new InMemorySiteManager([
    { id: '1', name: 'Cruce 1' },
    { id: '2', name: 'Cruce 2' }
  ]);

  mgr.delete('1');
  assert.strictEqual(mgr.getAll().length, 1);
  assert.strictEqual(mgr.getAll()[0].name, 'Cruce 2');
});

runTest('Gestor Cruces', 'Protección de invariante: no eliminar el último cruce', () => {
  const mgr = new InMemorySiteManager([{ id: '1', name: 'Cruce Único' }]);
  assert.throws(() => mgr.delete('1'), /Invariante/);
});

// --- SUITE 6: Estrés de Escala Vial (20 Cruces / 40 Semáforos) ---
runTest('Escalabilidad', 'Inserción masiva de 20 cruces viales independientes', () => {
  const mgr = new InMemorySiteManager([]);
  for (let i = 1; i <= 20; i++) {
    const pad = String(i).padStart(2, '0');
    mgr.save({
      name: `Cruce Km ${pad} · Tramo Vía al Llano`,
      location: `PR ${pad}+500 Calzada Principal`,
      p1: `👑 Maestro (Poste ${pad}-A)`,
      p2: `📡 Esclavo (Poste ${pad}-B)`
    });
  }
  const all = mgr.getAll();
  // 20 nuevos + 1 por defecto = 21 cruces (42 cabezas semafóricas)
  assert.strictEqual(all.length, 21);
});

runTest('Escalabilidad', 'Búsqueda instantánea y filtrado entre 20 cruces', () => {
  const mgr = new InMemorySiteManager([]);
  for (let i = 1; i <= 20; i++) {
    const pad = String(i).padStart(2, '0');
    mgr.save({
      name: `Cruce Km ${pad} · Sector Andino`,
      location: `PR ${pad}+000`
    });
  }
  const result = mgr.getAll('Km 17');
  assert.strictEqual(result.length, 1);
  assert.strictEqual(result[0].name, 'Cruce Km 17 · Sector Andino');
});

runTest('Escalabilidad', 'Edición atómica en flota de 20 cruces sin colisión', () => {
  const mgr = new InMemorySiteManager([]);
  const ids = [];
  for (let i = 1; i <= 20; i++) {
    const item = mgr.save({ name: `Cruce Base ${i}`, location: `Km ${i}` });
    ids.push(item.id);
  }
  const targetId = ids[12]; // Cruce 13
  mgr.save({ id: targetId, name: 'Cruce Base 13 MODIFICADO', location: 'Km 13+900' });

  const updated = mgr.getById(targetId);
  assert.strictEqual(updated.name, 'Cruce Base 13 MODIFICADO');
  assert.strictEqual(updated.location, 'Km 13+900');
  assert.strictEqual(mgr.getAll().length, 21);
});

runTest('Escalabilidad', 'Eliminación masiva secuencial de exactamente 17 de 20 cruces manteniendo integridad', () => {
  const mgr = new InMemorySiteManager([]); // inicia con 1 por defecto
  const ids = [];
  for (let i = 1; i <= 20; i++) {
    const item = mgr.save({ name: `Cruce Masivo Km ${i}`, location: `PR ${i}+000` });
    ids.push(item.id);
  }
  // Total antes de borrar: 20 nuevos + 1 base = 21 cruces
  assert.strictEqual(mgr.getAll().length, 21);

  // Eliminar exactamente 17 de los 20 cruces creados
  for (let i = 0; i < 17; i++) {
    const deleted = mgr.delete(ids[i]);
    assert.ok(deleted !== null, `Cruce ${ids[i]} eliminado con éxito`);
  }

  // Comprobar que quedan exactamente 4 cruces (3 de los 20 + 1 base)
  const remaining = mgr.getAll();
  assert.strictEqual(remaining.length, 4);

  // Comprobar que los cruces restantes 18, 19 y 20 siguen intactos
  assert.strictEqual(mgr.getById(ids[17]).name, 'Cruce Masivo Km 18');
  assert.strictEqual(mgr.getById(ids[18]).name, 'Cruce Masivo Km 19');
  assert.strictEqual(mgr.getById(ids[19]).name, 'Cruce Masivo Km 20');

  // Agregar 4 nuevos cruces adicionales
  const nuevos4 = [
    { name: 'Cruce Túnel La Línea', location: 'PR 50+100', p1: '👑 Maestro (Entrada)', p2: '📡 Esclavo (Salida)' },
    { name: 'Cruce Variante Guaduas', location: 'PR 62+300', p1: '👑 Maestro (Norte)', p2: '📡 Esclavo (Sur)' },
    { name: 'Cruce Paso Urbano Villeta', location: 'PR 75+800', p1: '👑 Maestro (P1)', p2: '📡 Esclavo (P2)' },
    { name: 'Cruce Puente Cundinamarca', location: 'PR 88+000', p1: '👑 Maestro (P1)', p2: '📡 Esclavo (P2)' }
  ];

  nuevos4.forEach(n => mgr.save(n));

  // La lista debe tener ahora exactamente 8 cruces (4 anteriores + 4 nuevos)
  const finalSites = mgr.getAll();
  assert.strictEqual(finalSites.length, 8);

  // Los 4 nuevos deben aparecer al inicio de la lista (orden LIFO / más recientes primero)
  assert.strictEqual(finalSites[0].name, 'Cruce Puente Cundinamarca');
  assert.strictEqual(finalSites[1].name, 'Cruce Paso Urbano Villeta');
  assert.strictEqual(finalSites[2].name, 'Cruce Variante Guaduas');
  assert.strictEqual(finalSites[3].name, 'Cruce Túnel La Línea');

  // RENOMBRAR TODOS LOS CRUCES REMANENTES (8 cruces)
  finalSites.forEach((site, index) => {
    mgr.save({
      id: site.id,
      name: `[ACTUALIZADO 2026] ${site.name}`,
      location: `${site.location} (Revisión Vial)`,
      p1: site.p1,
      p2: site.p2
    });
  });

  // Verificar que TODOS cambiaron su nombre y no se perdió ninguno
  const renamedSites = mgr.getAll();
  assert.strictEqual(renamedSites.length, 8);
  renamedSites.forEach(s => {
    assert.ok(s.name.startsWith('[ACTUALIZADO 2026]'), `Nombre actualizado correctamente: ${s.name}`);
    assert.ok(s.location.includes('(Revisión Vial)'), `Ubicación actualizada correctamente: ${s.location}`);
  });
});

// --- SUITE 7: D-26 - los avisos de la hora dicen QUE HACER ---
//
// La tabla que se prueba es la de js/avisos_equipo.js, la MISMA que carga index.html y
// que corre en el telefono: no se copia aqui ningun texto. Las tramas son las que compone
// el firmware -bluetooth_reportarAlarma() con el tramo de cada punta, y
// bluetooth_reportarEvento()-, pasadas por el parser de esta suite, que es el de la app.
const AvisosEquipo = require('./js/avisos_equipo.js');

function tramaCompleta(raw) {
  return `${raw}*${calcularChecksumNmea(raw.substring(1))}\r\n`;
}
function alarmaHora(node, causa) {
  const tramo = node === 'MAESTRO' ? 'RF:97%,RTT:70ms,SINRESP:0' : 'RX:1200,OK:300,RUIDO:2';
  return parseNmeaTelemetry(tramaCompleta(
    `$ALARM,NODE:${node},EVENTO:HORA_ESP32,CAUSA:${causa},${tramo},ACCION:SIGUE_SU_HORA,HORA:18:05:00`));
}
function eventoEquipo(node, origen, detalle) {
  return parseNmeaTelemetry(tramaCompleta(
    `$EVENT,NODE:${node},ORIGEN:${origen},DETALLE:${detalle},HORA:18:07:00`));
}
// El borde de CLAUDE.md 14, escrito: un numero con unidad de tiempo. Las constantes del
// firmware -cadencia de siembra, espera de la alarma, margen del cruce- no las puede
// recalcular esta app, asi que ninguna puede aparecer en un texto suyo.
const CIFRA_DE_TIEMPO = /\b\d+([.,]\d+)?\s*(ms|s|seg|segundos?|min|minutos?|h|horas?)\b/i;

['MAESTRO', 'ESCLAVO'].forEach(node => {
  runTest('Avisos D-26', `J17_MUDO del ${node}: revisar el circuito ESP32-STM32 de ESE poste`, () => {
    const a = AvisosEquipo.traducirAlarma(alarmaHora(node, 'J17_MUDO'));
    assert.ok(a, 'la app no traduce J17_MUDO');
    assert.match(a.texto, /^REVISE EL CIRCUITO ESP32-STM32 DE ESTE POSTE/);
    assert.ok(a.texto.includes(node), 'el texto no nombra el poste que avisa');
    assert.match(a.toast, /revise el circuito ESP32-STM32/i);
  });
  runTest('Avisos D-26', `RECHAZADA_FORMATO del ${node}: revisar el circuito, y el telefono NO lo arregla`, () => {
    const a = AvisosEquipo.traducirAlarma(alarmaHora(node, 'RECHAZADA_FORMATO'));
    assert.ok(a, 'la app no traduce RECHAZADA_FORMATO');
    assert.match(a.texto, /^REVISE EL CIRCUITO ESP32-STM32 DE ESTE POSTE/);
    assert.match(a.texto, /desde el telefono NO lo arregla/);
  });
  runTest('Avisos D-26', `SIN_HORA_DEL_ESP32 del ${node}: ponerle la hora desde el telefono en ESTE gabinete`, () => {
    const a = AvisosEquipo.traducirAlarma(alarmaHora(node, 'SIN_HORA_DEL_ESP32'));
    assert.ok(a, 'la app no traduce SIN_HORA_DEL_ESP32');
    assert.match(a.texto, /^PONGALE LA HORA DESDE EL TELEFONO AQUI, EN EL GABINETE DE ESTE POSTE/);
    assert.ok(!/REVISE EL CIRCUITO/.test(a.texto), 'manda al destornillador una averia que arregla el telefono');
    assert.match(a.toast, /Sincronizar/);
  });
});

runTest('Avisos D-26', 'Una causa de HORA_ESP32 que la app no conoce NO recibe la instruccion de otra', () => {
  assert.strictEqual(AvisosEquipo.traducirAlarma(alarmaHora('MAESTRO', 'CAUSA_QUE_NO_EXISTE')), null);
});

runTest('Avisos D-26', 'FALLO_RF: desde el Maestro, ir al Esclavo; desde el Esclavo, aqui', () => {
  const m = AvisosEquipo.traducirAlarma({ NODE: 'MAESTRO', EVENTO: 'FALLO_RF', CAUSA: 'REINTENTOS_AGOTADOS' });
  const e = AvisosEquipo.traducirAlarma({ NODE: 'ESCLAVO', EVENTO: 'FALLO_RF', CAUSA: 'SILENCIO_25000ms' });
  assert.match(m.texto, /^VAYA AL GABINETE DEL ESCLAVO/);
  assert.match(e.texto, /^PONGALE LA HORA DESDE EL TELEFONO AQUI/);
});

runTest('Avisos D-26', 'Los tres $EVENT nuevos (siembra, radio manda, salto por rojo) tienen texto', () => {
  const s = AvisosEquipo.traducirEvento(eventoEquipo('MAESTRO', 'ESP32', 'HORA_ESP32_SEMBRADA'));
  const i = AvisosEquipo.traducirEvento(eventoEquipo('ESCLAVO', 'ESP32', 'HORA_ESP32_IGNORADA_MANDA_RADIO'));
  const r = AvisosEquipo.traducirEvento(eventoEquipo('ESCLAVO', 'DEGRADADO', 'SALTO_DE_HORA_POR_ROJO'));
  assert.ok(s && /ha tomado la hora de su modulo ESP32/.test(s.texto));
  assert.ok(i && /^Normal:/.test(i.texto) && /Maestro por radio/.test(i.texto));
  assert.ok(r && /^COMPRUEBE LA HORA DE LOS DOS POSTES/.test(r.texto) && /ROJO/.test(r.texto));
  assert.strictEqual(r.tono, 'red');
  assert.strictEqual(AvisosEquipo.traducirEvento(eventoEquipo('MAESTRO', 'ESP32', 'DETALLE_NUEVO')), null);
});

runTest('Avisos D-26', 'Ningun texto de la tabla recita una cifra de tiempo del firmware (CLAUDE.md 14)', () => {
  const nodos = ['MAESTRO', 'ESCLAVO', undefined];
  const malos = [];
  Object.keys(AvisosEquipo.ALARMA).concat(Object.keys(AvisosEquipo.EVENTO)).forEach(k => {
    const tabla = k in AvisosEquipo.ALARMA ? AvisosEquipo.ALARMA : AvisosEquipo.EVENTO;
    nodos.forEach(n => {
      const e = typeof tabla[k] === 'function' ? tabla[k]({ NODE: n }) : tabla[k];
      [e.texto, e.toast || ''].forEach(t => { if (CIFRA_DE_TIEMPO.test(t)) malos.push(k + ': ' + t.match(CIFRA_DE_TIEMPO)[0]); });
    });
  });
  // El detector tiene que saber fallar, o este verde no dice nada.
  assert.ok(CIFRA_DE_TIEMPO.test('cada 5 min') && CIFRA_DE_TIEMPO.test('tras 25 s') &&
            !CIFRA_DE_TIEMPO.test('el conector J17 del ESP32'), 'el detector de cifras no distingue');
  assert.deepStrictEqual(malos, []);
});

// --- SUITE 8: D-23 - el diagnostico de radio de la punta a la que estas conectado ---
//
// La logica que se prueba es la de js/diagnostico_enlace.js, la MISMA que carga
// index.html. Las tramas se componen con el formato que emite el firmware -el snprintf
// de bluetooth_reportarEvento() del Esclavo- y se parten con el parser de la app, no con
// uno de esta suite: app_12_un_solo_parser existe para que eso no vuelva a pasar.
const DiagnosticoEnlace = require('./js/diagnostico_enlace.js');
const fs = require('fs');
const path = require('path');

function diagRadio(rx, ok, ruido, node) {
  return eventoEquipo(node || 'ESCLAVO', 'ENLACE_RF',
                      `RX:${rx} OK:${ok} RUIDO:${ruido}`);
}

runTest('D-23 diagnostico', 'el periodico del Esclavo se reconoce y se leen sus tres contadores', () => {
  DiagnosticoEnlace.olvidar();
  const d = diagRadio(1200, 300, 2);
  assert.ok(DiagnosticoEnlace.esPeriodico(d), 'no reconoce el periodico de D-23');
  assert.deepStrictEqual(DiagnosticoEnlace.leer(d), { rx: 1200, ok: 300, ruido: 2 });
});

// EL CORAZON DEL ARREGLO. Si esto se pone rojo, el periodico esta volviendo a la lista
// de 30 y la bitacora se vuelve a comer los $ALARM en quince minutos.
runTest('D-23 diagnostico', 'los OTROS $EVENT del mismo ORIGEN NO son el periodico: son sucesos y van a la bitacora', () => {
  DiagnosticoEnlace.olvidar();
  // La vuelta del enlace del Esclavo y los tres cambios de estado del Maestro. Los
  // literales son los de los snprintf de las dos puntas.
  ['RECUPERADO_OK:300_RUIDO:2', 'PERDIDO_RF:80', 'SIN_MEDIDA_RF:--', 'OK_RF:97']
    .forEach(det => {
      assert.ok(!DiagnosticoEnlace.esPeriodico(eventoEquipo('ESCLAVO', 'ENLACE_RF', det)),
                `se traga como periodico un SUCESO del enlace: ${det}`);
    });
  // Y un $EVENT de otro origen tampoco, aunque su detalle empezara por RX:.
  assert.ok(!DiagnosticoEnlace.esPeriodico(eventoEquipo('ESCLAVO', 'RELOJ', 'RX:1 OK:1 RUIDO:0')),
            'mira el DETALLE sin mirar el ORIGEN');
});

runTest('D-23 diagnostico', 'el delta se mide contra la muestra anterior y nombra su ventana', () => {
  DiagnosticoEnlace.olvidar();
  const a = DiagnosticoEnlace.ver(diagRadio(1000, 200, 0), 1000);
  assert.strictEqual(a.delta, null, 'inventa un delta en la primera muestra');
  assert.ok(a.primera, 'no marca la primera muestra');
  const b = DiagnosticoEnlace.ver(diagRadio(1300, 250, 3), 31000);
  assert.deepStrictEqual(b.delta, { rx: 300, ok: 50, ruido: 3, ventanaMs: 30000 });
});

// N-144 en esta pantalla: un contador que baja no es un delta negativo.
runTest('D-23 diagnostico', 'un contador que BAJA se lee como reinicio del poste, no como delta negativo', () => {
  DiagnosticoEnlace.olvidar();
  DiagnosticoEnlace.ver(diagRadio(9000, 900, 5), 1000);
  const r = DiagnosticoEnlace.ver(diagRadio(12, 2, 0), 31000);
  assert.ok(r.reinicio, 'no detecta que el poste rearranco');
  assert.strictEqual(r.delta, null, 'resta contra la cuenta del arranque anterior');
  assert.ok(r.avisos.some(a => /SE HA REINICIADO/.test(a.texto) && a.tono === 'red'),
            'un poste que rearranca solo no deja aviso rojo');
});

// LA REGLA DE ESTE FICHERO, MEDIDA: las lineas las gobiernan los sucesos, no el reloj.
runTest('D-23 diagnostico', 'en regimen NO gasta bitacora: solo anota cuando algo CAMBIA', () => {
  DiagnosticoEnlace.olvidar();
  let ms = 1000;
  // La primera si deja una linea: que el diagnostico empezo a llegar hay que decirlo.
  assert.strictEqual(DiagnosticoEnlace.ver(diagRadio(1000, 100, 0), ms).avisos.length, 1);
  // Veinte muestras seguidas de radio sana -diez minutos- y ni una linea mas.
  let lineas = 0;
  for (let i = 1; i <= 20; i++) {
    ms += 30000;
    lineas += DiagnosticoEnlace.ver(diagRadio(1000 + i * 300, 100 + i * 50, 0), ms).avisos.length;
  }
  assert.strictEqual(lineas, 0,
    'el periodico sigue gastando lineas de las 30 en regimen: la bitacora se llena sola');
});

runTest('D-23 diagnostico', 'el ruido se anota cuando EMPIEZA y cuando PARA, una vez por episodio', () => {
  DiagnosticoEnlace.olvidar();
  let ms = 1000;
  DiagnosticoEnlace.ver(diagRadio(1000, 100, 0), ms);
  // Tres ventanas seguidas comiendo ruido: una sola linea, la de que empieza.
  const empieza = DiagnosticoEnlace.ver(diagRadio(1300, 150, 4), ms += 30000);
  assert.ok(empieza.avisos.some(a => /EMPIEZA a descartar/.test(a.texto)), 'no avisa del ruido');
  assert.strictEqual(DiagnosticoEnlace.ver(diagRadio(1600, 200, 9), ms += 30000).avisos.length, 0,
    'repite el aviso de ruido en cada muestra: eso es volver a llenar la bitacora');
  assert.strictEqual(DiagnosticoEnlace.ver(diagRadio(1900, 250, 14), ms += 30000).avisos.length, 0);
  // Y cuando para, se dice: si no, el tecnico no sabe que el episodio se cerro.
  const para = DiagnosticoEnlace.ver(diagRadio(2200, 300, 14), ms += 30000);
  assert.ok(para.avisos.some(a => /deja de descartar/.test(a.texto)), 'no avisa de que el ruido paro');
});

runTest('D-23 diagnostico', 'una radio que no recibe NADA se anota, y se distingue del ruido', () => {
  DiagnosticoEnlace.olvidar();
  let ms = 1000;
  DiagnosticoEnlace.ver(diagRadio(1000, 100, 0), ms);
  const mudo = DiagnosticoEnlace.ver(diagRadio(1000, 100, 0), ms += 30000);
  assert.ok(mudo.avisos.some(a => /NO recibió un solo byte/.test(a.texto) && a.tono === 'red'),
            'una radio muda no deja rastro');
  // Y al volver el trafico se dice tambien.
  const vuelve = DiagnosticoEnlace.ver(diagRadio(1300, 150, 0), ms += 30000);
  assert.ok(vuelve.avisos.some(a => /Vuelven a entrar bytes/.test(a.texto)));
});

// La regla de RegistroEnlace aplicada aqui: no medido NO es cero.
runTest('D-23 diagnostico', 'sin tramas en la ventana el % de ruido es null, NUNCA 0', () => {
  assert.strictEqual(DiagnosticoEnlace.pctRuido(null), null, 'sin delta devuelve un numero');
  assert.strictEqual(DiagnosticoEnlace.pctRuido({ rx: 40, ok: 0, ruido: 0, ventanaMs: 30000 }), null,
    'entraron bytes sueltos y ninguna trama: un 0% diria que todas llegaron bien');
  assert.strictEqual(DiagnosticoEnlace.pctRuido({ rx: 400, ok: 3, ruido: 1, ventanaMs: 30000 }), 25);
});

// CLAUDE.md 8: la app no fija umbrales sobre una radio que no ha medido. El firmware lo
// decidio y lo dejo escrito; esto es el trinquete que impide que vuelva por la app.
runTest('D-23 diagnostico', 'no hay ningun umbral ni veredicto escrito en el modulo', () => {
  const fuente = fs.readFileSync(path.join(__dirname, 'js', 'diagnostico_enlace.js'), 'utf8');
  // Se mira el CODIGO, no los comentarios: aqui los comentarios citan lo que explican
  // -"RF < 70% = degradado"- y un grep sin filtrar los cuenta (CLAUDE.md 7.1).
  const codigo = fuente.replace(/^\s*\/\/.*$/gm, '');
  assert.ok(!/UMBRAL|DEGRADADO|ACEPTABLE/i.test(codigo), 'ha entrado un veredicto por umbral');
  // El detector tiene que saber fallar.
  assert.ok(/UMBRAL/i.test('const UMBRAL_RUIDO = 30;'), 'el detector de umbrales no detecta');
});

runTest('D-23 diagnostico', 'al soltar el enlace se olvidan los contadores del poste anterior', () => {
  DiagnosticoEnlace.olvidar();
  DiagnosticoEnlace.ver(diagRadio(9000, 900, 5), 1000);
  DiagnosticoEnlace.olvidar();
  assert.strictEqual(DiagnosticoEnlace.resumen(), null, 'se queda con la muestra del poste anterior');
  const primera = DiagnosticoEnlace.ver(diagRadio(50, 5, 0), 31000);
  assert.ok(primera.primera && !primera.reinicio,
    'la primera del poste nuevo se resta contra la del anterior y finge un reinicio');
});

// --- SUITE 9: la barrera retenida por la camara (14/09) ---
//
// La logica que se prueba es la de js/aviso_camara_pluma.js, la MISMA que carga
// index.html. Las tramas se componen con el formato que emiten los snprintf de
// {Maestro,Esclavo}/src/botones.cpp y se parten con el parser de la app.
const AvisoCamaraPluma = require('./js/aviso_camara_pluma.js');

function eventoCamara(detalle, node) {
  return eventoEquipo(node || 'MAESTRO', 'CAMARA_PLUMA', detalle);
}

runTest('Camara-pluma', 'los dos DETALLE del firmware se reconocen y se leen', () => {
  AvisoCamaraPluma.olvidar();
  assert.deepStrictEqual(AvisoCamaraPluma.leer(eventoCamara('VETO_SOSTENIDO_S:180')),
    { clase: 'SOSTENIDO', segundos: 180, fueraDeCota: false, vetos: null });
  assert.deepStrictEqual(AvisoCamaraPluma.leer(eventoCamara('VETO_ACTUADO_N:7')),
    { clase: 'ACTUADO', segundos: null, fueraDeCota: false, vetos: 7 });
});

// EL '!' ES UN VALOR DEL PROTOCOLO, NO UN ERROR DE FORMATO (N-154). Y es el caso MAS
// grave -- la retencion mas larga --: si el patron no lo admitiera, justo ese aviso
// caeria al camino del crudo y el tecnico no veria cartel.
runTest('Camara-pluma', "el '!' de fuera de cota se admite y NO se convierte en un cero", () => {
  AvisoCamaraPluma.olvidar();
  const l = AvisoCamaraPluma.leer(eventoCamara('VETO_SOSTENIDO_S:!'));
  assert.ok(l && l.fueraDeCota === true, "no reconoce el '!' del firmware");
  assert.strictEqual(l.segundos, null, 'un 0 diria "lleva cero segundos", que es lo contrario');
  AvisoCamaraPluma.ver(eventoCamara('VETO_SOSTENIDO_S:!'));
  const v = AvisoCamaraPluma.vigente();
  assert.ok(/más tiempo del que el equipo puede publicar/.test(v.medida), v.medida);
  // La hora del equipo SI lleva digitos y es legitima; lo que no puede aparecer es una
  // cuenta de segundos, que es justo el dato que el equipo dijo que no cabia.
  assert.ok(!/\d+\s*s\b/.test(v.medida), 'se ha colado una cifra de segundos inventada');
});

runTest('Camara-pluma', 'lo que la tabla no nombra se devuelve null y sigue por el camino del crudo', () => {
  AvisoCamaraPluma.olvidar();
  // Un DETALLE nuevo del mismo ORIGEN, y un prefijo con basura detras: los dos tienen
  // que caer fuera. Un patron sin anclar se los tragaria en el cartel equivocado.
  ['VETO_SOSTENIDO_MIN:4', 'VETO_SOSTENIDO_S:180 EXTRA', 'CAM_NUEVA:1', '']
    .forEach(det => assert.strictEqual(AvisoCamaraPluma.ver(eventoCamara(det)), null, det));
  // Y el mismo DETALLE con otro ORIGEN tampoco: se miran los dos campos.
  assert.strictEqual(AvisoCamaraPluma.ver(eventoEquipo('MAESTRO', 'RELOJ', 'VETO_SOSTENIDO_S:180')), null);
  assert.strictEqual(AvisoCamaraPluma.vigente(), null, 'ha abierto cartel sin aviso valido');
});

// EL CONTROL QUE SEPARA "la camara hizo su trabajo" de "la camara tiene la barrera
// secuestrada". Un cartel que saltara con cada vehiculo se aprende a ignorar.
runTest('Camara-pluma', 'un veto que ACTUA es lo normal: linea de bitacora, pero NO cartel', () => {
  AvisoCamaraPluma.olvidar();
  const v = AvisoCamaraPluma.ver(eventoCamara('VETO_ACTUADO_N:3'));
  assert.ok(v && v.abre === false && v.linea && v.toast === null, 'el flanco abre cartel');
  assert.strictEqual(AvisoCamaraPluma.vigente(), null, 'un veto normal ha abierto cartel');
});

runTest('Camara-pluma', 'BASTA UNA VEZ: la primera retencion sostenida abre el cartel y gasta UNA linea', () => {
  AvisoCamaraPluma.olvidar();
  const v = AvisoCamaraPluma.ver(eventoCamara('VETO_SOSTENIDO_S:120'));
  assert.ok(v.abre && v.linea && v.linea.tono === 'red' && v.toast, 'la primera no abre');
  const c = AvisoCamaraPluma.vigente();
  assert.ok(c && c.bajada === false && c.segundos === 120, JSON.stringify(c));
});

// EL CORAZON DEL ARREGLO, gemelo del de D-23: el firmware REPITE mientras dure, y una
// linea por repeticion volveria a comerse las 30 entradas de la bitacora.
runTest('Camara-pluma', 'las repeticiones del episodio refrescan el cartel y NO gastan bitacora', () => {
  AvisoCamaraPluma.olvidar();
  AvisoCamaraPluma.ver(eventoCamara('VETO_SOSTENIDO_S:90'));
  for (let i = 2; i <= 8; i++) {
    const v = AvisoCamaraPluma.ver(eventoCamara('VETO_SOSTENIDO_S:' + (90 * i)));
    assert.ok(!v.abre && v.linea === null && v.toast === null, 'la repeticion ' + i + ' gasta linea');
  }
  assert.strictEqual(AvisoCamaraPluma.vigente().segundos, 720, 'el cartel no sigue la ultima medida');
});

// QUIEN DICE QUE YA BAJO ES EL $STATUS, no el silencio del equipo: el firmware no
// publica un evento de fin, y "deja de repetir" es indistinguible de un cable cortado.
runTest('Camara-pluma', 'el cartel NO se retira al bajar la barrera, pero deja de decir que esta arriba', () => {
  AvisoCamaraPluma.olvidar();
  AvisoCamaraPluma.ver(eventoCamara('VETO_SOSTENIDO_S:200'));
  assert.strictEqual(AvisoCamaraPluma.verPluma('ARRIBA'), null, 'ARRIBA cierra algo');
  assert.strictEqual(AvisoCamaraPluma.verPluma(null), null, '"no tengo el dato" se lee como "bajo"');
  const linea = AvisoCamaraPluma.verPluma('ABAJO');
  assert.ok(linea && linea.tono === 'green', 'el soltarse no deja linea');
  const c = AvisoCamaraPluma.vigente();
  assert.ok(c && c.bajada === true, 'el cartel se ha retirado solo');
  assert.ok(!/está ARRIBA/.test(c.accion) && /revise el apunte/.test(c.accion), c.accion);
  assert.ok(/estuvo retenida/.test(c.medida) && !/lleva retenida/.test(c.medida), c.medida);
  // Y no repite la linea en cada $STATUS siguiente: es una transicion, no una cadencia.
  assert.strictEqual(AvisoCamaraPluma.verPluma('ABAJO'), null, 'repite la linea de bajada');
});

// 🔴 LO QUE LA APP NO PUEDE DECIR NUNCA. El equipo no ve imagen (D-12) y no separa un
// vehiculo parado de una camara mal apuntada: los dos son un contacto cerrado. Afirmar
// la averia inventa un dato que el equipo no tiene, y manda a NO mirar debajo del brazo.
runTest('Camara-pluma', 'ningun texto afirma que la camara este averiada, y se dice por que no se puede saber', () => {
  const AVERIA = /averi|estropead|dañad|rota|defectuos|fallo de la cámara/i;
  AvisoCamaraPluma.olvidar();
  const textos = [];
  textos.push(AvisoCamaraPluma.ver(eventoCamara('VETO_ACTUADO_N:1')).linea.texto);
  ['VETO_SOSTENIDO_S:90', 'VETO_SOSTENIDO_S:!'].forEach(det => {
    AvisoCamaraPluma.olvidar();
    const v = AvisoCamaraPluma.ver(eventoCamara(det));
    textos.push(v.linea.texto, v.toast);
    const c = AvisoCamaraPluma.vigente();
    textos.push(c.titulo, c.medida, c.accion, c.limite);
    textos.push(AvisoCamaraPluma.verPluma('ABAJO').texto);
    const d = AvisoCamaraPluma.vigente();
    textos.push(d.titulo, d.medida, d.accion);
  });
  // El detector sabe fallar, o este verde no dice nada.
  assert.ok(AVERIA.test('la camara esta averiada') && !AVERIA.test('revise el apunte'),
            'el detector de averia no distingue');
  const malos = textos.filter(t => AVERIA.test(t || ''));
  assert.deepStrictEqual(malos, []);
  assert.ok(/no ve imagen/.test(AvisoCamaraPluma.LIMITE) &&
            /Quien juzga es usted/.test(AvisoCamaraPluma.LIMITE), AvisoCamaraPluma.LIMITE);
});

// SIN CIFRAS DEL FIRMWARE (CLAUDE.md 14): el unico numero que sale es el que VIENE EN LA
// TRAMA. Ni el todo-rojo maximo, ni la cadencia del aviso, ni el tope del buffer.
runTest('Camara-pluma', 'ningun texto FIJO recita una cifra de tiempo del firmware', () => {
  const fijos = [AvisoCamaraPluma.ACCION, AvisoCamaraPluma.LIMITE];
  AvisoCamaraPluma.olvidar();
  fijos.push(AvisoCamaraPluma.ver(eventoCamara('VETO_ACTUADO_N:1')).linea.texto);
  AvisoCamaraPluma.olvidar();
  AvisoCamaraPluma.ver(eventoCamara('VETO_SOSTENIDO_S:60'));
  fijos.push(AvisoCamaraPluma.vigente().titulo, AvisoCamaraPluma.verPluma('ABAJO').texto,
             AvisoCamaraPluma.vigente().titulo, AvisoCamaraPluma.vigente().accion);
  assert.ok(CIFRA_DE_TIEMPO.test('pasados 90 s'), 'el detector de cifras no distingue');
  assert.deepStrictEqual(fijos.filter(t => CIFRA_DE_TIEMPO.test(t || '')), []);
});

runTest('Camara-pluma', 'al soltar el enlace se olvida: el cartel es de UN poste', () => {
  AvisoCamaraPluma.olvidar();
  AvisoCamaraPluma.ver(eventoCamara('VETO_SOSTENIDO_S:150', 'ESCLAVO'));
  assert.ok(/POSTE 2 \(ESCLAVO\)/.test(AvisoCamaraPluma.vigente().medida));
  AvisoCamaraPluma.olvidar();
  assert.strictEqual(AvisoCamaraPluma.vigente(), null, 'el cartel del poste anterior sobrevive');
  // Y sin NODE no se adivina: atribuirlo al poste equivocado manda a abrir el gabinete
  // que no es. Misma regla que _cual() de js/avisos_equipo.js.
  AvisoCamaraPluma.ver(parseNmeaTelemetry(tramaCompleta(
    '$EVENT,ORIGEN:CAMARA_PLUMA,DETALLE:VETO_SOSTENIDO_S:99,HORA:18:07:00')));
  assert.ok(/no dice cuál/.test(AvisoCamaraPluma.vigente().medida),
            AvisoCamaraPluma.vigente().medida);
});

// index.html tiene que CARGAR el modulo, o esta suite estaria midiendo un fichero que en
// el telefono no corre. Es la misma comprobacion que protege a js/avisos_equipo.js.
runTest('Camara-pluma', 'index.html carga js/aviso_camara_pluma.js ANTES de app.js', () => {
  const html = fs.readFileSync(path.join(__dirname, 'index.html'), 'utf8');
  const iMod = html.indexOf('js/aviso_camara_pluma.js');
  const iApp = html.indexOf('src="app.js"');
  assert.ok(iMod > 0, 'index.html no carga js/aviso_camara_pluma.js');
  assert.ok(iMod < iApp, 'se carga despues de app.js: en el telefono no existiria al arrancar');
});

// =============================================================================
// RESUMEN FINAL
// =============================================================================
console.log('='.repeat(80));
console.log(` RESUMEN DE PRUEBAS: ${passedTests} PASS | ${failedTests} FALLAS  (Total: ${totalTests})`);
console.log('='.repeat(80));

if (failedTests > 0) {
  process.exit(1);
} else {
  console.log(' 🎉 TODAS LAS PRUEBAS UNITARIAS PASARON AL 100%');
  process.exit(0);
}
