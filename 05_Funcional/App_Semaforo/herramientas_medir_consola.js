// Mide la ERGONOMIA de la pantalla de operacion, no su ancho.
//
// El ancho ya lo mide herramientas_medir_desborde.js. Esto contesta a las dos
// preguntas que una auditoria de HMI planteo el 04/09 y que NO se pueden contestar
// mirando el CSS, porque dependen del alto del telefono y de como parta el texto:
//
//   1. ¿Hay que hacer SCROLL entre ver el semaforo y pulsar un mando? Si el operario
//      pierde de vista las luces para dar la orden, no puede confirmar que paso.
//   2. ¿Se PARTEN las palabras dentro de los botones? "AUTO-/MATIC-/O" en tres
//      trozos no se lee a pleno sol y con guantes.
//
// NO PULSA NADA: solo carga y mide, para no despertar ningun prompt().
const path = require('path');
const fs = require('fs');
const puppeteer = require('puppeteer-core');

const CHROME_PATH = 'C:/Program Files/Google/Chrome/Application/chrome.exe';
const EDGE_PATH = 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe';
const executablePath = fs.existsSync(CHROME_PATH) ? CHROME_PATH : EDGE_PATH;
const APP = 'file://' + path.join(__dirname, 'index.html');

// Alto UTIL, que no es el alto del telefono: hay que descontar la barra del sistema
// y la del navegador. Se usan los altos utiles reales, no los nominales.
const PANTALLAS = [
  [320, 568, 'iPhone SE 1a gen'],
  [360, 640, 'Android comun (Galaxy A, Moto G)'],
  [390, 664, 'iPhone 12-15'],
  [412, 732, 'la maqueta de las capturas'],
];

(async () => {
  const browser = await puppeteer.launch({
    executablePath, headless: 'new', args: ['--no-sandbox', '--disable-gpu'],
  });

  let veredicto = 0;
  for (const [w, h, desc] of PANTALLAS) {
   for (const conectado of [false, true]) {
    const page = await browser.newPage();
    await page.setViewport({ width: w, height: h, deviceScaleFactor: 2 });
    await page.goto(APP, { waitUntil: 'domcontentloaded' });
    await new Promise(r => setTimeout(r, 400));

    // SE MIDEN LOS DOS ESTADOS, y el segundo es el que vive el operario.
    //
    // Sin punta identificada la app ensena LAS DOS emergencias -ROJO TOTAL del Maestro y
    // AMBAR EMERGENCIA del Esclavo-, porque no sabe con cual habla. En cuanto llega el
    // primer $STATUS oculta la que no toca. Medir solo el primer estado publica una
    // pantalla mas larga que la que se usa; medir solo el segundo esconde lo que ve quien
    // abre la app antes de conectar. Se publican los dos.
    if (conectado) {
      await page.evaluate(() => {
        const b = document.getElementById('btn-op-ambar-emergencia');
        if (b) b.style.display = 'none';   // punta = MAESTRO
      });
      await new Promise(r => setTimeout(r, 120));
    }

    const r = await page.evaluate(() => {
      const vh = window.innerHeight;

      // El semaforo: se busca por el contenedor de las lamparas del poste 1.
      const lampara = document.querySelector('.tl-housing, .traffic-light, .semaforo, [class*="lamp"], [class*="poste"]');
      const sem = lampara ? lampara.getBoundingClientRect() : null;

      // 🔴 SEGUNDA VEZ QUE ESTE CENSO SE QUEDA CORTO, Y LA REGLA ES LA MISMA.
      //
      // Aqui habia cuatro ids a mano. El ultimo mando de la pantalla no es ninguno de
      // ellos: es 'btn-op-ambar-emergencia', un boton ancho que termina 134 px mas abajo
      // que el cuarto. Con la lista escrita a mano, la herramienta publicaba "31 px
      // fuera" sobre una pantalla que se salia 165. Un censo por lista es una lista que
      // alguien tiene que acordarse de actualizar, y nadie se acuerda.
      //
      // Se recorren TODOS los .pad-btn que esten VISIBLES. El filtro de visibilidad
      // importa: la app oculta uno de los dos mandos de emergencia segun la punta
      // -ROJO TOTAL es del Maestro, AMBAR EMERGENCIA del Esclavo-, asi que medir los
      // ocultos daria una pantalla mas larga que la que ve nadie.
      const visibles = [...document.querySelectorAll('.pad-btn')]
        .filter(b => b.offsetParent !== null);

      // LO DIARIO Y EL RESTO, SEPARADOS, porque el criterio del responsable los separa:
      // "sin desplazamiento deberia ver y operar el operador; al bajar, lo que no usa a
      // diario". Medirlos juntos daba un numero que no contesta a esa frase.
      //
      // DIARIO = los mandos de trafico: automatico, dar paso, ambar, y la emergencia de
      // SU punta -la app oculta la de la otra-. NO diario = VOLVER AL MENU, que sale del
      // modo, y CANCELAR AMBAR, que solo aparece con un ambar puesto.
      const NO_DIARIO = ['btn-op-menu', 'btn-op-cancelar-ambar'];
      const botones = visibles.filter(b => !NO_DIARIO.includes(b.id));
      const otros = visibles.filter(b => NO_DIARIO.includes(b.id));
      const cajas = botones.map(b => b.getBoundingClientRect());
      const finOtros = otros.length
        ? Math.round(Math.max(...otros.map(o => o.getBoundingClientRect().bottom))) : null;

      // 1. ¿Cabe todo en una pantalla?
      const finUltimoMando = cajas.length ? Math.max(...cajas.map(c => c.bottom)) : null;

      // 🔴 Y EL TECHO NO ES EL BORDE DE LA PANTALLA: ES LA BARRA QUE FLOTA ENCIMA.
      //
      // ESTE INSTRUMENTO DIO "SIN HALLAZGOS" SOBRE UNA PANTALLA CON UN MANDO TAPADO, y
      // lo destapo una captura de campo del 04/09: la barra de pestañas -Trafico /
      // Eventos- esta en position fixed y se dibuja POR ENCIMA de la botonera. En la
      // foto se ve un boton amarillo asomando por arriba y por abajo de la barra.
      //
      // Medir contra window.innerHeight da por bueno todo lo que este dentro del
      // viewport, y un boton puede estar dentro Y ESTAR CUBIERTO. Es §4 sobre el propio
      // instrumento: el buscador respondia, y aun asi no sabia encontrar.
      //
      // El techo util es el borde SUPERIOR de la barra flotante, no el del viewport. Se
      // busca la barra por su posicion calculada -fixed o sticky- y no por su clase: una
      // lista de selectores escrita a mano es una lista que alguien tiene que acordarse
      // de actualizar.
      let techoUtil = vh, tapador = null;
      for (const el of document.querySelectorAll('body *')) {
        if (el.offsetParent === null && getComputedStyle(el).position !== 'fixed') continue;
        const cs = getComputedStyle(el);
        if (cs.position !== 'fixed' && cs.position !== 'sticky') continue;
        const c = el.getBoundingClientRect();
        // Solo cuenta lo que flota ABAJO y ocupa ancho: una barra, no un icono suelto.
        if (!c.height || c.width < 0.5 * window.innerWidth) continue;
        if (c.bottom < vh * 0.6) continue;
        if (c.top < techoUtil) { techoUtil = c.top; tapador = el.className || el.id || el.tagName; }
      }

      // 2. ¿Se parten las palabras?
      //
      // 🔴 LA PRIMERA VERSION DE ESTO SE QUEDO CORTA Y HAY QUE DECIRLO: miraba solo el
      // <strong> de CUATRO botones buscados por id. Las fotos del 04/09 mostraban ademas
      // "AMBAR/EMERG/ENCIA" -otro id-, "MAESTR/O" y "Ciclo autonom/o" -los <small>, que
      // ni se miraban-. Un instrumento que mira una parte y da un veredicto sobre el todo
      // es peor que no tenerlo: publica "ninguna" sobre una pantalla llena de ellas.
      //
      // Ahora se recorren TODOS los .pad-btn del documento y TODOS sus nodos de texto.
      // El criterio: una palabra se ha partido si el elemento ocupa mas lineas que
      // palabras tiene. Es aproximado por arriba -un texto de 2 palabras en 2 lineas no
      // se marca aunque una este partida- y por eso se publica tambien el numero de
      // lineas, que es lo que se puede contrastar contra la foto.
      const partidas = [];
      for (const b of document.querySelectorAll('.pad-btn')) {
        for (const s of b.querySelectorAll('strong, small, span, div')) {
          const txt = (s.textContent || '').trim();
          if (!txt || s.children.length) continue;
          const cs = getComputedStyle(s);
          const lh = parseFloat(cs.lineHeight) || parseFloat(cs.fontSize) * 1.2;
          const caja = s.getBoundingClientRect();
          if (!caja.height) continue;
          const lineas = Math.round(caja.height / lh);
          const palabras = txt.split(/\s+/).length;
          if (lineas > palabras) {
            partidas.push({ texto: txt.slice(0, 34), lineas, palabras,
                            ancho: Math.round(caja.width) });
          }
        }
      }

      return {
        vh, techoUtil, tapador,
        semTop: sem ? Math.round(sem.top) : null,
        semBottom: sem ? Math.round(sem.bottom) : null,
        finUltimoMando: finUltimoMando === null ? null : Math.round(finUltimoMando),
        alto: Math.round(document.documentElement.scrollHeight),
        partidas,
        nBotones: botones.length,
        finOtros,
      };
    });

    // El techo es la barra flotante si la hay, no el borde de la pantalla.
    const scrollParaMandar = r.finUltimoMando !== null && r.finUltimoMando > r.techoUtil;
    const px = r.finUltimoMando === null ? null : r.finUltimoMando - r.techoUtil;

    console.log(`\n== ${w}x${h}  ${desc} ==`);
    console.log(`   alto del documento        : ${r.alto} px  (viewport ${r.vh})`);
    console.log(`   semaforo                  : ${r.semTop} .. ${r.semBottom} px`);
    console.log(`   techo util (barra flotante): ${r.techoUtil} px` +
      (r.tapador ? `  <- la tapa "${r.tapador}"` : '  (no hay barra flotante)'));
    console.log(`   ultimo mando DIARIO en    : ${r.finUltimoMando} px  ` +
      (scrollParaMandar ? `-> ${px} px TAPADO O FUERA` : '-> VISIBLE Y PULSABLE'));
    console.log(`   lo no diario termina en   : ${r.finOtros} px  (puede quedar debajo)`);
    if (r.partidas.length) {
      for (const p of r.partidas) {
        console.log(`   PALABRA PARTIDA           : "${p.texto}" en ${p.lineas} lineas ` +
                    `(${p.palabras} palabra(s), ancho ${p.ancho} px)`);
      }
    } else {
      console.log('   palabras partidas         : ninguna');
    }
    if ((scrollParaMandar && conectado) || r.partidas.length) veredicto = 1;
    await page.close();
   }
  }

  await browser.close();
  console.log(`\nVEREDICTO: ${veredicto ? 'HAY HALLAZGOS' : 'sin hallazgos'}`);
  process.exit(veredicto);
})();
