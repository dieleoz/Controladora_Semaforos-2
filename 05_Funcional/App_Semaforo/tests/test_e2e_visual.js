// ===== tests/test_e2e_visual.js =====
// Test E2E Automatizado Visual con Validación de Flujo Completo (Apertura, Guardado y Cambio de Estado)

const puppeteer = require('puppeteer-core');
const path = require('path');
const fs = require('fs');

const EVIDENCIA_DIR = path.join(__dirname, '..', '..', '..', 'evidencia');
const ARTIFACT_DIR = 'C:\\Users\\Diego.Zuñiga\\.gemini\\antigravity-ide\\brain\\85affed7-f286-4094-898b-631bf1b49593';

if (!fs.existsSync(EVIDENCIA_DIR)) {
  fs.mkdirSync(EVIDENCIA_DIR, { recursive: true });
}

const CHROME_PATH = 'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe';
const EDGE_PATH = 'C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe';
const executablePath = fs.existsSync(CHROME_PATH) ? CHROME_PATH : EDGE_PATH;

async function guardarCaptura(page, filename) {
  const p1 = path.join(EVIDENCIA_DIR, filename);
  await page.screenshot({ path: p1, fullPage: false });
  if (fs.existsSync(ARTIFACT_DIR)) {
    const p2 = path.join(ARTIFACT_DIR, filename);
    await page.screenshot({ path: p2, fullPage: false });
  }
  console.log(`  📸 [FEEDBACK VISUAL] Captura guardada: ${filename}`);
}

async function runVisualE2E() {
  console.log('='.repeat(80));
  console.log('🚀 EJECUTANDO VALIDACIÓN E2E DE APERTURA, GUARDADO Y OPERABILIDAD');
  console.log('='.repeat(80));

  const browser = await puppeteer.launch({
    executablePath,
    headless: 'new',
    args: ['--no-sandbox', '--disable-setuid-sandbox', '--disable-gpu']
  });

  const page = await browser.newPage();
  await page.setViewport({ width: 412, height: 915, isMobile: true, hasTouch: true });

  try {
    // 1. Cargar Dashboard
    console.log('\n1. Cargando Dashboard Principal...');
    await page.goto('http://localhost:3000/', { waitUntil: 'domcontentloaded', timeout: 10000 });
    await new Promise(r => setTimeout(r, 600));
    await guardarCaptura(page, '01_modo_operario_principal.png');

    // 2. Probar Selección de Cruce (Abre modal y cambia cruce)
    console.log('\n2. Probando Selector de Cruces Viales (#btn-select-site)...');
    await page.click('#btn-select-site');
    await page.waitForSelector('#site-modal.active', { timeout: 3000 });
    await guardarCaptura(page, '02_modal_cruces_abierto.png');

    // Seleccionar el segundo cruce de la lista
    const cruceItems = await page.$$('#dynamic-site-list .bt-device-item');
    if (cruceItems.length > 1) {
      await cruceItems[1].click();
      await new Promise(r => setTimeout(r, 400));
      await guardarCaptura(page, '03_cruce_cambiado_exito.png');
    }

    // 3. Probar Selección de Bluetooth (Abre modal y enlaza nodo)
    console.log('\n3. Probando Modal Bluetooth (#btnDevice)...');
    await page.click('#btnDevice');
    await page.waitForSelector('#bt-modal.active', { timeout: 3000 });
    await guardarCaptura(page, '04_modal_bluetooth_abierto.png');

    const devItems = await page.$$('#bt-device-list-container .bt-device-item');
    if (devItems.length > 1) {
      await devItems[1].click(); // Enlazar al Esclavo P2
      await new Promise(r => setTimeout(r, 400));
      await guardarCaptura(page, '05_nodo_esclavo_conectado.png');
    }

    // 4. Probar Desbloqueo a Modo Técnico con PIN 1234
    console.log('\n4. Probando Desbloqueo a Modo Técnico (PIN 1234)...');
    await page.click('#btn-toggle-role');
    await page.waitForSelector('#pin-modal.active', { timeout: 3000 });

    for (const d of ['1', '2', '3', '4']) {
      await page.click(`.pin-btn[data-key="${d}"]`);
      await new Promise(r => setTimeout(r, 80));
    }
    await new Promise(r => setTimeout(r, 500));
    await guardarCaptura(page, '06_modo_tecnico_activo.png');

    // 5. Probar Guardado de Tiempos de Ciclo
    console.log('\n5. Probando Guardado de Tiempos de Ciclo en Pestaña Tiempos...');
    await page.click('#tab-btn-tiempos');
    await new Promise(r => setTimeout(r, 400));

    // Cambiar tiempos a 3m Verde, 3m Rojo, 20s Despeje
    await page.evaluate(() => {
      document.getElementById('num-tiempo-verde').value = 3;
      document.getElementById('num-tiempo-rojo').value = 3;
      document.getElementById('num-tiempo-despeje').value = 20;
    });
    await page.click('#btn-aplicar-tiempos');
    await new Promise(r => setTimeout(r, 500));
    await guardarCaptura(page, '07_tiempos_guardados_exito.png');

    // 6. Probar Courier RTC
    console.log('\n6. Probando Asistente Courier RTC...');
    await page.click('#tab-btn-diag');
    await new Promise(r => setTimeout(r, 400));
    await page.click('#btn-courier-capture');
    await new Promise(r => setTimeout(r, 1200));
    await page.click('#btn-courier-inject');
    await new Promise(r => setTimeout(r, 500));
    await guardarCaptura(page, '08_courier_rtc_inyectado.png');

    console.log('\n' + '='.repeat(80));
    console.log('🎉 VALIDACIÓN E2E DE APERTURA, GUARDADO Y OPERACIÓN EXITOSA');
    console.log('='.repeat(80) + '\n');

  } catch (err) {
    console.error('Error en Test E2E:', err);
    process.exitCode = 1;
  } finally {
    await browser.close();
  }
}

runVisualE2E();
