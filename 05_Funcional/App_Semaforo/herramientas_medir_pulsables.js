// Mide EL TAMANO DE LO QUE SE PULSA. Nadie lo medía.
//
// Ya hay tres instrumentos sobre esta interfaz y ninguno contesta a esto:
//   herramientas_medir_desborde.js  -> el ANCHO: que no se salga de la pantalla
//   herramientas_medir_consola.js   -> el ALTO: que no haya que bajar para mandar
//   test_funcional_app.py           -> el COLOR: los ratios WCAG del propio CSS
//
// Falta la cuarta dimension, y es la que decide si una orden sale o no sale: si el
// dedo acierta. Un boton legible que mide 30 px de alto se lee perfectamente y se
// falla, y en un cruce fallar el boton no es una molestia -es una orden que no se
// dio, o peor, la de al lado-.
//
// EL UMBRAL Y DE DONDE SALE, para que no sea una opinion:
//   - WCAG 2.2, criterio 2.5.8 "Target Size (Minimum)", nivel AA: 24x24 px CSS.
//   - WCAG 2.2, criterio 2.5.5 "Target Size (Enhanced)", nivel AAA: 44x44 px CSS.
//   - Material Design y las guias de Android: 48x48 dp.
//
// SE EXIGE 44x44, el AAA, y el motivo va escrito porque no es el umbral habitual:
// esta app no se usa sentado. Se usa de pie, en la calle, a pleno sol, con guantes y
// con trafico esperando. El AA de 24 px esta pensado para un raton. Los 44 px son el
// ancho de una yema, y aqui la yema puede llevar guante.
//
// LO QUE ESTE INSTRUMENTO NO MIDE, y va escrito para que no se lea como permiso:
//   - que el boton correcto este a mano: mide tamano, no colocacion.
//   - el guante de verdad. 44 px es el minimo publicado; que baste con guante grueso
//     lo dice quien lo pruebe en el poste, no un navegador sin cabeza.
//   - el temblor y la prisa. Un objetivo que se aprueba justo en 44 px se falla mas
//     que uno de 60; por eso se imprime la MEDIDA de cada uno, no solo el veredicto.
//
// NO PULSA NADA: solo carga y mide, para no despertar ningun prompt().
const path = require('path');
const fs = require('fs');
const puppeteer = require('puppeteer-core');

const CHROME_PATH = 'C:/Program Files/Google/Chrome/Application/chrome.exe';
const EDGE_PATH = 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe';
const executablePath = fs.existsSync(CHROME_PATH) ? CHROME_PATH : EDGE_PATH;
const APP = 'file://' + path.join(__dirname, 'index.html');

const MINIMO = 44;   // WCAG 2.2 AAA (2.5.5). El porque, arriba.
const SEPARACION = 8; // px entre dos objetivos contiguos; por debajo, el dedo duda.

// EL ANCHO MAS ESTRECHO ES EL QUE MANDA. Un boton que cumple a 412 px puede no
// cumplir a 320: el alto de un boton con texto CRECE al estrecharse -el texto pasa a
// dos lineas- pero el ancho se reparte entre mas columnas y ENCOGE. Se miden los dos
// extremos y se publica el peor, que es §4.ter: una medida a un solo ancho no vale.
const PANTALLAS = [
  [320, 568, 'iPhone SE 1a gen'],
  [412, 732, 'la maqueta de las capturas'],
];

(async () => {
  const browser = await puppeteer.launch({
    executablePath, headless: 'new', args: ['--no-sandbox', '--disable-gpu'],
  });

  let veredicto = 0;
  for (const [w, h, desc] of PANTALLAS) {
    const page = await browser.newPage();
    await page.setViewport({ width: w, height: h, deviceScaleFactor: 2 });
    await page.goto(APP, { waitUntil: 'domcontentloaded' });
    await new Promise(r => setTimeout(r, 400));

    const r = await page.evaluate((MINIMO, SEPARACION) => {
      // CENSO, NO LISTA. Es la leccion que este instrumento ya se cobro dos veces en
      // herramientas_medir_consola.js: una lista de ids escrita a mano es una lista
      // que alguien tiene que acordarse de actualizar, y nadie se acuerda. Se recorre
      // TODO lo pulsable del documento.
      const PULSABLES = 'button, a[href], input, select, [role="button"], [data-cmd], [data-tab]';

      const visible = el => {
        if (el.offsetParent === null) return false;
        const cs = getComputedStyle(el);
        return cs.visibility !== 'hidden' && cs.display !== 'none';
      };

      // Las pestanas ocultan secciones enteras: sin abrirlas, el censo solo veria la
      // pestana activa y publicaria "todo bien" sobre una cuarta parte de la app.
      // Se abren todas ANTES de medir -sin pulsar: se quita la clase a mano-.
      for (const s of document.querySelectorAll('.tab-content')) {
        s.classList.add('active');
        s.style.display = 'block';
      }

      const items = [...document.querySelectorAll(PULSABLES)].filter(visible);

      const pequenos = [];
      const cajas = [];
      for (const el of items) {
        const c = el.getBoundingClientRect();
        if (!c.width || !c.height) continue;
        const etiqueta = (el.getAttribute('aria-label') || el.textContent || el.id ||
                          el.tagName).trim().replace(/\s+/g, ' ').slice(0, 38);
        cajas.push({ c, etiqueta, id: el.id || '' });
        if (c.width < MINIMO || c.height < MINIMO) {
          pequenos.push({ etiqueta, id: el.id || '',
                          w: Math.round(c.width), h: Math.round(c.height) });
        }
      }

      // SEPARACION entre objetivos contiguos. Dos botones pegados de 44 px cumplen el
      // tamano y aun asi se fallan: el dedo tapa los dos. Solo se comparan pares que
      // se solapan en el otro eje -si no, no son vecinos, estan en otra fila-.
      const juntos = [];
      for (let i = 0; i < cajas.length; i++) {
        for (let j = i + 1; j < cajas.length; j++) {
          const a = cajas[i].c, b = cajas[j].c;
          const solapaY = a.top < b.bottom && b.top < a.bottom;
          const solapaX = a.left < b.right && b.left < a.right;
          if (solapaX && solapaY) continue;   // anidados o superpuestos: otro caso
          let hueco = null;
          if (solapaY) hueco = b.left >= a.right ? b.left - a.right
                                                 : (a.left >= b.right ? a.left - b.right : null);
          else if (solapaX) hueco = b.top >= a.bottom ? b.top - a.bottom
                                                     : (a.top >= b.bottom ? a.top - b.bottom : null);
          if (hueco !== null && hueco < SEPARACION) {
            juntos.push({ a: cajas[i].etiqueta, b: cajas[j].etiqueta,
                          hueco: Math.round(hueco) });
          }
        }
      }

      const alturas = cajas.map(x => Math.round(x.c.height)).sort((p, q) => p - q);
      return { total: cajas.length, pequenos, juntos,
               menorAlto: alturas[0], medianaAlto: alturas[Math.floor(alturas.length / 2)] };
    }, MINIMO, SEPARACION);

    console.log(`\n== ${w}x${h}  ${desc} ==`);
    console.log(`   objetivos pulsables censados : ${r.total}`);
    console.log(`   alto menor / mediana         : ${r.menorAlto} / ${r.medianaAlto} px`);
    if (r.pequenos.length) {
      console.log(`   POR DEBAJO DE ${MINIMO}x${MINIMO} px : ${r.pequenos.length}`);
      for (const p of r.pequenos) {
        console.log(`      ${String(p.w).padStart(4)}x${String(p.h).padEnd(4)} "${p.etiqueta}"` +
                    (p.id ? `  #${p.id}` : ''));
      }
      veredicto = 1;
    } else {
      console.log(`   todos llegan a ${MINIMO}x${MINIMO} px : si`);
    }
    if (r.juntos.length) {
      console.log(`   PARES A MENOS DE ${SEPARACION} px      : ${r.juntos.length}`);
      for (const p of r.juntos.slice(0, 12)) {
        console.log(`      ${p.hueco} px entre "${p.a}" y "${p.b}"`);
      }
      veredicto = 1;
    }
    await page.close();
  }

  await browser.close();
  console.log(`\nVEREDICTO: ${veredicto ? 'HAY HALLAZGOS' : 'sin hallazgos'}`);
  process.exit(veredicto);
})();
