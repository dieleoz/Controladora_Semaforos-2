// Mide desbordamiento HORIZONTAL de la app a varios anchos de telefono.
// No pulsa nada: solo carga y mide, para no despertar ningun prompt().
const path = require('path');
const puppeteer = require('puppeteer-core');
const fs = require('fs');
const CHROME_PATH = 'C:/Program Files/Google/Chrome/Application/chrome.exe';
const EDGE_PATH = 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe';
const executablePath = fs.existsSync(CHROME_PATH) ? CHROME_PATH : EDGE_PATH;

const APP = 'file://' + path.join(__dirname, 'index.html');

const ANCHOS = [
  [320, 'iPhone SE 1a gen / Android muy estrecho'],
  [360, 'Android comun (Galaxy A, Moto G)'],
  [390, 'iPhone 12-15'],
  [412, 'la maqueta de las capturas'],
];

(async () => {
  const browser = await puppeteer.launch({ executablePath, headless: 'new', args: ['--no-sandbox','--disable-gpu'] });
  for (const [w, desc] of ANCHOS) {
    const page = await browser.newPage();
    await page.setViewport({ width: w, height: 780, deviceScaleFactor: 2 });
    await page.goto(APP, { waitUntil: 'domcontentloaded' });
    await new Promise(r => setTimeout(r, 400));

    const r = await page.evaluate(() => {
      const doc = document.documentElement;
      const desborde = doc.scrollWidth - doc.clientWidth;
      // Que elementos se salen del viewport por la derecha
      const culpables = [];
      document.querySelectorAll('*').forEach(el => {
        const b = el.getBoundingClientRect();
        if (b.width > 0 && b.right > window.innerWidth + 0.5) {
          const id = el.id ? '#' + el.id : '';
          const cls = el.className && typeof el.className === 'string'
            ? '.' + el.className.trim().split(/\s+/).slice(0, 2).join('.') : '';
          culpables.push(`${el.tagName.toLowerCase()}${id}${cls} (+${Math.round(b.right - window.innerWidth)}px)`);
        }
      });
      // Y los cuatro botones tacticos, uno a uno
      const pad = ['btn-op-auto', 'btn-op-step', 'btn-op-amber', 'btn-op-emergency'].map(id => {
        const el = document.getElementById(id);
        if (!el) return `${id}: NO EXISTE`;
        const b = el.getBoundingClientRect();
        const cortado = b.right > window.innerWidth + 0.5;
        return `${id}: ${Math.round(b.left)}..${Math.round(b.right)} ${cortado ? 'CORTADO' : 'ok'}`;
      });
      return { desborde, culpables: [...new Set(culpables)].slice(0, 8), pad };
    });

    console.log(`\n${w}px  (${desc})`);
    console.log(`  desbordamiento horizontal: ${r.desborde}px  ${r.desborde > 0 ? '<-- SE SALE' : 'ok'}`);
    r.pad.forEach(p => console.log('    ' + p));
    if (r.culpables.length) {
      console.log('  elementos que se salen:');
      r.culpables.forEach(c => console.log('    ' + c));
    }
    await page.close();
  }
  await browser.close();
})();
