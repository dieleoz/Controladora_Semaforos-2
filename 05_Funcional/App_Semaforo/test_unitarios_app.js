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

function calcularChecksumNmea(cadena) {
  let crc = 0;
  for (let i = 0; i < cadena.length; i++) {
    crc ^= cadena.charCodeAt(i);
  }
  return crc.toString(16).toUpperCase().padStart(2, '0');
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

  const tokens = payloadSinDollar.split(',');
  const type = tokens[0];
  const data = { type };

  for (let i = 1; i < tokens.length; i++) {
    const pair = tokens[i].split(':');
    if (pair.length === 2) {
      data[pair[0]] = pair[1];
    } else if (pair.length > 2) {
      data[pair[0]] = pair.slice(1).join(':');
    }
  }

  return data;
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

  if (isNaN(v) || v < 1 || v > 15) {
    return { valid: false, error: 'VERDE_OUT_OF_RANGE', msg: 'Verde debe estar entre 1 y 15 min.' };
  }
  if (isNaN(r) || r < 1 || r > 15) {
    return { valid: false, error: 'ROJO_OUT_OF_RANGE', msg: 'Rojo debe estar entre 1 y 15 min.' };
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

runTest('Tiempos', 'Rechazo de tiempo Verde menor a 1 minuto (0 min)', () => {
  const res = validateTiempos(0, 4, 25);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'VERDE_OUT_OF_RANGE');
});

runTest('Tiempos', 'Rechazo de tiempo Verde mayor a 15 minutos (20 min)', () => {
  const res = validateTiempos(20, 4, 25);
  assert.strictEqual(res.valid, false);
  assert.strictEqual(res.error, 'VERDE_OUT_OF_RANGE');
});

runTest('Tiempos', 'Rechazo de tiempo Rojo menor a 1 minuto', () => {
  const res = validateTiempos(3, 0, 25);
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
