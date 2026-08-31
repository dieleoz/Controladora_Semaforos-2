// ===== Simulaciones/puente_esp32/arnes_app.js =====
//
// LA PUNTA DE LA APP DEL SIMULADOR DEL PUENTE — CODIGO REAL, NO UNA IMITACION.
//
// Este fichero NO reimplementa ni el compositor de comandos ni el parser de tramas.
// Carga `index.html`, sus `js/*.js` y `app.js` REALES en jsdom -exactamente como hace
// test_dom_execution.js, que es el instrumento que ya los ejercita- y los expone como
// un proceso que habla por su entrada y su salida estandar. El simulador de Python es
// quien mueve los bytes.
//
// Las dos puertas por las que la app habla con el equipo son las que la propia app
// usa, y son suyas, no de este arnes:
//
//   IDA    window.bluetoothSerial.write(rawCmd, ...)   <- lo compone enviarComandoFirmware()
//   VUELTA window._btSubscribeCb(chunk)                <- lo consume parseNmeaTelemetry()
//
// LO QUE SI ES MODELO EN ESTA PUNTA, Y ES SU UNICO PUNTO CIEGO:
//
//   El PLUGIN DE CORDOVA. `bluetoothSerial.subscribe('\n', cb)` no entrega bytes: el
//   plugin acumula y llama al callback UNA VEZ POR TROZO TERMINADO EN EL DELIMITADOR.
//   Aqui eso se reproduce con un buffer y un split por '\n'. Es un modelo -no es
//   codigo de la app ni del plugin- y esta escrito asi a proposito, porque es
//   EXACTAMENTE la pieza que decide si partir o unir tramas rompe algo: la app no ve
//   bytes, ve trozos, y quien los recorta es el plugin.
//
// El resto -localStorage, navigator.vibrate, prompt- son sustitutos del navegador,
// copiados del arnes de DOM que ya vive en 05_Funcional/App_Semaforo.
//
// PROTOCOLO DE LINEA, hacia el simulador de Python:
//   CMD <orden>   pulsa el boton [data-cmd="<orden>"] REAL y devuelve lo que la app
//                 haya escrito por el canal serie, como TX <hex>
//   BTN <id>      pulsa el boton por id de elemento (para los que no llevan data-cmd)
//   RXHEX <hex>   entrega esos bytes por el canal serie, respetando el troceado del
//                 plugin, y devuelve el estado observable del tablero
//   DOM           devuelve el estado observable sin tocar nada
//   PIN           teclea 1234 en el teclado REAL si el modal esta abierto
//   QUIT

const fs = require('fs');
const path = require('path');

const APP = path.join(__dirname, '..', '..', '..', '05_Funcional', 'App_Semaforo');

let JSDOM;
try {
  ({ JSDOM } = require(path.join(APP, 'node_modules', 'jsdom')));
} catch (e) {
  // ABORTADO, no PASS: sin jsdom esta punta no puede ejercer nada, y decirlo es el
  // trabajo del arnes. Un arnes que se callara dejaria al simulador midiendo media
  // cadena y publicando una cuenta entera.
  process.stdout.write('ABORT no se pudo cargar jsdom desde App_Semaforo/node_modules: '
                       + e.message + '\n');
  process.exit(2);
}

const htmlContent = fs.readFileSync(path.join(APP, 'index.html'), 'utf8');
const jsContent = fs.readFileSync(path.join(APP, 'app.js'), 'utf8');

const dom = new JSDOM(htmlContent, {
  runScripts: 'dangerously', resources: 'usable', url: 'http://localhost/'
});
const { window } = dom;
const { document } = window;

const storage = {};
window.localStorage = {
  getItem: (k) => storage[k] || null,
  setItem: (k, v) => { storage[k] = String(v); },
  removeItem: (k) => { delete storage[k]; },
  clear: () => { Object.keys(storage).forEach(k => delete storage[k]); }
};
window.navigator.vibrate = () => true;
window.prompt = () => null;

// La app escribe por consola en cada TX. Aqui la salida estandar ES el canal del
// protocolo, asi que su ruido se desvia a la de error: no se silencia -si la app
// revienta, quiero verlo- pero no se mezcla con las lineas que el simulador parsea.
['log', 'info', 'warn', 'error', 'debug'].forEach(n => {
  window.console[n] = (...a) => process.stderr.write('[app] ' + a.join(' ') + '\n');
});

// --- LA IDA: se captura lo que la app compone, sin tocarlo -------------------------
let escrituras = [];
// --- LA VUELTA: el troceado del plugin de Cordova, que es el unico modelo de aqui --
let bufferPlugin = '';
let delimitador = '\n';

window.bluetoothSerial = {
  isEnabled: (ok) => ok(),
  list: (ok) => ok([{ name: 'JDY-31 Maestro', id: '00:11:22:33:44:55',
                      address: '00:11:22:33:44:55' }]),
  connect: (mac, ok) => ok(),
  disconnect: (ok) => ok && ok(),
  subscribe: (delim, cb) => { delimitador = delim; window._btSubscribeCb = cb; },
  write: (data, ok) => { escrituras.push(data); if (ok) ok(); }
};

// Carga en el MISMO orden que index.html declara, igual que test_dom_execution.js: un
// `const X` dentro de eval() no crea global, y sin exponerlos app.js no encuentra
// SiteManager (N-75).
let initError = null;
try {
  (htmlContent.match(/src="(js\/[^"]+\.js)"/g) || [])
    .map(m => m.replace(/src="|"/g, ''))
    .forEach(rel => {
      const src = fs.readFileSync(path.join(APP, rel), 'utf8');
      const nombres = (src.match(/^(?:const|class|function|var|let)\s+([A-Za-z_$][\w$]*)/gm) || [])
        .map(m => m.split(/\s+/)[1]);
      window.eval(src + ';' + nombres.map(n => `try{window.${n}=${n};}catch(e){}`).join(''));
    });
  window.eval(jsContent);
  document.dispatchEvent(new window.Event('DOMContentLoaded'));
} catch (e) {
  initError = e;
}
if (initError) {
  process.stdout.write('ABORT app.js no se pudo evaluar: ' + initError.message + '\n');
  process.exit(2);
}

// Conexion por el camino REAL de la app: se abre su modal y se pulsa su equipo. Sin
// esto la app no llama a subscribe() y no hay canal de vuelta que medir.
document.getElementById('btnDevice').click();
const primerEquipo = document.querySelector('.bt-device-item');
if (primerEquipo) primerEquipo.click();
if (typeof window._btSubscribeCb !== 'function') {
  process.stdout.write('ABORT la app no se suscribio al canal serie: sin eso no hay '
                       + 'camino de vuelta que medir\n');
  process.exit(2);
}

function tecleaPin() {
  const modal = document.getElementById('pin-modal');
  if (!modal || !modal.classList.contains('active')) return false;
  ['1', '2', '3', '4'].forEach(d => {
    const b = document.querySelector(`.pin-btn[data-key="${d}"]`);
    if (b) b.click();
  });
  return !modal.classList.contains('active');
}

function hex(s) {
  let r = '';
  for (let i = 0; i < s.length; i++) {
    r += s.charCodeAt(i).toString(16).toUpperCase().padStart(2, '0');
  }
  return r;
}

function desdeHex(h) {
  let r = '';
  for (let i = 0; i + 1 < h.length; i += 2) r += String.fromCharCode(parseInt(h.substr(i, 2), 16));
  return r;
}

function volcarTx() {
  escrituras.forEach(w => process.stdout.write('TX ' + hex(w) + '\n'));
  escrituras = [];
}

function texto(id) {
  const el = document.getElementById(id);
  return el ? (el.textContent || '').replace(/\s+/g, ' ').trim() : '(no existe)';
}

function estadoTablero() {
  const badge = document.getElementById('badge-modo');
  const log = document.getElementById('event-feed');
  return {
    modo: badge ? (badge.textContent || '').trim() : '?',
    fase: texto('phase-desc'),
    contador: texto('cd-num'),
    rf: texto('rf-quality'),
    nodo: texto('node-name'),
    eventos: log ? log.children.length : -1,
    ultimo: log && log.children.length ? (log.children[0].textContent || '')
      .replace(/\s+/g, ' ').trim().slice(0, 120) : ''
  };
}

// EL TROCEADO DEL PLUGIN. Ver la cabecera: es el unico modelo de esta punta.
function entregar(bytes) {
  bufferPlugin += bytes;
  let trozos = 0;
  let i;
  while ((i = bufferPlugin.indexOf(delimitador)) >= 0) {
    const trozo = bufferPlugin.slice(0, i + delimitador.length);
    bufferPlugin = bufferPlugin.slice(i + delimitador.length);
    trozos++;
    try {
      window._btSubscribeCb(trozo);
    } catch (e) {
      process.stdout.write('EXC ' + e.message + '\n');
    }
  }
  return trozos;
}

let pendiente = '';
process.stdin.setEncoding('utf8');
process.stdin.on('data', (chunk) => {
  pendiente += chunk;
  let i;
  while ((i = pendiente.indexOf('\n')) >= 0) {
    const linea = pendiente.slice(0, i).replace(/\r$/, '');
    pendiente = pendiente.slice(i + 1);
    atender(linea);
  }
});
process.stdin.on('end', () => process.exit(0));

function atender(linea) {
  if (linea.startsWith('CMD ')) {
    const orden = linea.slice(4);
    escrituras = [];
    const btn = document.querySelector(`[data-cmd="${orden}"]`);
    if (!btn) { process.stdout.write('ERR sin boton data-cmd=' + orden + '\n'); return; }
    btn.click();
    tecleaPin();          // si la app pide PIN, se teclea por su teclado REAL
    volcarTx();
    process.stdout.write('OK ' + orden + '\n');

  } else if (linea.startsWith('BTN ')) {
    const el = document.getElementById(linea.slice(4));
    if (!el) { process.stdout.write('ERR sin elemento ' + linea.slice(4) + '\n'); return; }
    escrituras = [];
    el.click();
    tecleaPin();
    volcarTx();
    process.stdout.write('OK ' + linea.slice(4) + '\n');

  } else if (linea.startsWith('RXHEX ')) {
    const trozos = entregar(desdeHex(linea.slice(6)));
    const e = estadoTablero();
    process.stdout.write('DOM ' + JSON.stringify(e) + '\n');
    process.stdout.write('OK trozos=' + trozos + ' pendiente=' + bufferPlugin.length + '\n');

  } else if (linea === 'DOM') {
    process.stdout.write('DOM ' + JSON.stringify(estadoTablero()) + '\n');
    process.stdout.write('OK\n');

  } else if (linea === 'PIN') {
    process.stdout.write('OK pin=' + tecleaPin() + '\n');

  } else if (linea === 'QUIT') {
    process.exit(0);

  } else if (linea.length === 0) {
    return;

  } else {
    process.stdout.write('ERR orden desconocida: ' + linea + '\n');
  }
}

process.stdout.write('LISTO APP delimitador=' + hex(delimitador) + '\n');
