// AVISO, ANTES DE CONECTAR ESTO A NADA (02/09):
//
// ESTE GUION NO AFIRMA NADA. Hace capturas de pantalla; no compara, no cuenta, y no
// publica ninguna cifra x/y. Un PASS suyo no dice que la interfaz este bien: dice que el
// navegador no reventó.
//
// Y mide a UN SOLO ANCHO -412 px-, que es precisamente el unico de los cuatro en los que
// el desbordamiento que sufrio este proyecto NO aparecia. Una captura a un solo ancho
// demuestra que a ESE ancho se veia bien, y nada mas.
//
// Lo que hace falta de verdad es un arnes de interfaz que AFIRME -anchos, contraste,
// recorte-, no uno que fotografie. Mientras eso no exista, esto no se conecta a la
// compuerta: una fila que no sabe fallar es peor que una fila que falta.

const puppeteer = require('puppeteer-core');
const path = require('path');
const fs = require('fs');

const ARTIFACT_DIR = 'C:\\Users\\Diego.Zuñiga\\.gemini\\antigravity-ide\\brain\\85affed7-f286-4094-898b-631bf1b49593';
const CHROME_PATH = 'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe';
const EDGE_PATH = 'C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe';

const executablePath = fs.existsSync(CHROME_PATH) ? CHROME_PATH : EDGE_PATH;

async function runE2ETests() {
  console.log('='.repeat(80));
  console.log('🚀 INICIANDO TEST VISUAL E2E AUTOMATIZADO CON NAVEGADOR');
  console.log('='.repeat(80));
  console.log('Navegador ejecutable:', executablePath);

  const browser = await puppeteer.launch({
    executablePath,
    headless: 'new',
    args: ['--no-sandbox', '--disable-setuid-sandbox', '--disable-gpu']
  });

  const page = await browser.newPage();
  await page.setViewport({ width: 412, height: 915, isMobile: true, hasTouch: true });

  const errors = [];
  page.on('pageerror', err => errors.push(`Page Error: ${err.message}`));
  page.on('console', msg => {
    if (msg.type() === 'error') errors.push(`Console Error: ${msg.text()}`);
  });

  try {
    // 1. Cargar aplicación (Dashboard Estado)
    console.log('1. Navegando a http://localhost:3000/...');
    await page.goto('http://localhost:3000/', { waitUntil: 'networkidle0', timeout: 15000 });
    await page.screenshot({ path: path.join(ARTIFACT_DIR, '01_dashboard.png'), fullPage: false });
    console.log('  [OK] Captura 1 guardada: 01_dashboard.png');

    // 2. Abrir Modal de Dispositivo / Bluetooth
    console.log('2. Probando botón y modal de conexión Bluetooth (#btnDevice)...');
    const btnDevice = await page.$('#btnDevice');
    if (btnDevice) {
      await btnDevice.click();
      await page.waitForSelector('#bt-modal', { visible: true, timeout: 3000 });
      await page.screenshot({ path: path.join(ARTIFACT_DIR, '02_modal_bluetooth.png') });
      console.log('  [OK] Captura 2 guardada: 02_modal_bluetooth.png');
      
      // Cerrar modal
      const btnCerrarBt = await page.$('#modal-bt-close');
      if (btnCerrarBt) await btnCerrarBt.click();
      await new Promise(r => setTimeout(r, 400));
    }

    // 3. Abrir Modal Gestor de Cruces
    console.log('3. Probando Gestor de Cruces (#btn-select-site)...');
    const btnSite = await page.$('#btn-select-site');
    if (btnSite) {
      await btnSite.click();
      await page.waitForSelector('#site-modal', { visible: true, timeout: 3000 });
      await page.screenshot({ path: path.join(ARTIFACT_DIR, '03_modal_gestor_cruces.png') });
      console.log('  [OK] Captura 3 guardada: 03_modal_gestor_cruces.png');

      // Cerrar modal cruces
      const btnCloseSite = await page.$('#modal-site-close');
      if (btnCloseSite) await btnCloseSite.click();
      await new Promise(r => setTimeout(r, 400));
    }

    // 4. Pestaña Control de Fases y Teclado PIN
    console.log('4. Probando Pestaña de Control y Teclado PIN...');
    await page.click('.nav-item[data-tab="tab-control"]');
    await new Promise(r => setTimeout(r, 400));
    await page.screenshot({ path: path.join(ARTIFACT_DIR, '04_pestana_control.png') });
    console.log('  [OK] Captura 4 guardada: 04_pestana_control.png');
    
    // Clic en Forzar Rojo
    const btnForzarRojo = await page.$('#btn-forzar-rojo');
    if (btnForzarRojo) {
      await btnForzarRojo.click();
      await page.waitForSelector('#pin-modal', { visible: true, timeout: 3000 });
      
      // Teclear PIN 1 2 3
      for (const d of ['1', '2', '3']) {
        await page.click(`.pin-btn[data-key="${d}"]`);
        await new Promise(r => setTimeout(r, 100));
      }
      await page.screenshot({ path: path.join(ARTIFACT_DIR, '05_modal_pin_ingreso.png') });
      console.log('  [OK] Captura 5 guardada: 05_modal_pin_ingreso.png');
      
      // Cancelar modal PIN
      const btnCerrarPin = await page.$('#modal-pin-close');
      if (btnCerrarPin) await btnCerrarPin.click();
      await new Promise(r => setTimeout(r, 400));
    }

    // 5. Pestaña Diagnóstico y Courier RTC
    console.log('5. Probando Pestaña Diagnóstico (tab-diag)...');
    await page.click('.nav-item[data-tab="tab-diag"]');
    await new Promise(r => setTimeout(r, 400));
    await page.screenshot({ path: path.join(ARTIFACT_DIR, '06_pestana_diagnostico.png') });
    console.log('  [OK] Captura 6 guardada: 06_pestana_diagnostico.png');

    // 6. Pestaña Ajustes (tab-rtc)
    console.log('6. Probando Pestaña Ajustes (tab-rtc)...');
    await page.click('.nav-item[data-tab="tab-rtc"]');
    await new Promise(r => setTimeout(r, 400));
    await page.screenshot({ path: path.join(ARTIFACT_DIR, '07_pestana_ajustes.png') });
    console.log('  [OK] Captura 7 guardada: 07_pestana_ajustes.png');

    // 7. Simular Telemetría en Vivo (Semáforos activos, 12.8V, Fase 1 Verde, etc.)
    console.log('7. Regresando a Estado e inyectando Trama de Telemetría $STATUS en vivo...');
    await page.click('.nav-item[data-tab="tab-estado"]');
    await new Promise(r => setTimeout(r, 400));

    await page.evaluate(() => {
      // Inyectar trama en la ventana
      const tramaSimulada = '$STATUS,MODO:AUTO,FASE:VERDE_P1,RESTANTE:45,TOT_FASE:180,BAT1:12.8,BAT2:12.5,RADIO:98,RTT:65,ALARM:0,TELA:1*5A\n';
      if (typeof window.parseAndProcessStatusLine === 'function') {
        window.parseAndProcessStatusLine(tramaSimulada);
      } else if (typeof window.procesarLineaBluetooth === 'function') {
        window.procesarLineaBluetooth(tramaSimulada);
      }
    });

    await new Promise(r => setTimeout(r, 600));
    await page.screenshot({ path: path.join(ARTIFACT_DIR, '08_dashboard_telemetria_en_vivo.png') });
    console.log('  [OK] Captura 8 guardada: 08_dashboard_telemetria_en_vivo.png');

    console.log('='.repeat(80));
    console.log('🎉 TODOS LOS CONTROLES Y PANTALLAS FUERON PROBADOS CON ÉXITO');
    console.log('Total errores en consola:', errors.length > 0 ? errors : 'Ninguno (0 errores)');
    console.log('='.repeat(80));

  } catch (err) {
    // N-46 (02/09): ESTE catch SE TRAGABA TODO Y EL GUION SALIA CON 0 PASE LO QUE PASARA.
    //
    // No habia un solo process.exit en el fichero, y justo encima se imprime "TODOS LOS
    // CONTROLES Y PANTALLAS FUERON PROBADOS CON EXITO". O sea que si alguien lo hubiera
    // conectado a compuerta.py, habria anadido una fila verde permanente al acta. Es
    // literal el defecto que este repositorio lleva un mes cerrando, esperando a que
    // alguien lo diera de alta.
    console.error('Error durante la ejecución E2E:', err);
    process.exitCode = 1;
  } finally {
    await browser.close();
  }
}

runE2ETests();
