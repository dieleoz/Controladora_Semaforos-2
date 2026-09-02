// ===== test_dom_execution.js =====
// Test de ejecucion real del DOM y JavaScript (app.js + index.html) usando jsdom.
// Comprueba que no haya excepciones TypeError, referencias a null, tabs rotos o cuelgues por tramas.

const fs = require('fs');
const path = require('path');
const { JSDOM } = require('jsdom');

let testsPassed = 0;
let testsFailed = 0;

function assert(condition, message) {
  if (condition) {
    console.log(`  [OK] ${message}`);
    testsPassed++;
  } else {
    console.error(`  [FAIL] ${message}`);
    testsFailed++;
    process.exitCode = 1;
  }
}

console.log('='.repeat(80));
console.log(' 🧪 EJECUCIÓN REAL DEL DOM Y JAVASCRIPT CON JSDOM (APP.JS)');
console.log('='.repeat(80));

const htmlPath = path.join(__dirname, 'index.html');
const jsPath = path.join(__dirname, 'app.js');

const htmlContent = fs.readFileSync(htmlPath, 'utf8');
const jsContent = fs.readFileSync(jsPath, 'utf8');

// Configurar entorno DOM
const dom = new JSDOM(htmlContent, {
  runScripts: 'dangerously',
  resources: 'usable',
  url: 'http://localhost/'
});

const { window } = dom;
const { document } = window;

// Mock de localStorage
const storage = {};
window.localStorage = {
  getItem: (k) => storage[k] || null,
  setItem: (k, v) => { storage[k] = String(v); },
  removeItem: (k) => { delete storage[k]; },
  clear: () => { Object.keys(storage).forEach(k => delete storage[k]); }
};

// Mock de navigator
window.navigator.vibrate = (pattern) => true;

// Mock de bluetoothSerial nativo (Cordova)
let sentFrames = [];
window.bluetoothSerial = {
  isEnabled: (ok, fail) => ok(),
  list: (ok, fail) => ok([
    { name: 'JDY-31 Maestro', id: '00:11:22:33:44:55', address: '00:11:22:33:44:55' },
    { name: 'HC-05 Esclavo', id: '66:77:88:99:AA:BB', address: '66:77:88:99:AA:BB' }
  ]),
  connect: (mac, ok, fail) => ok(),
  disconnect: (ok) => ok && ok(),
  subscribe: (delimiter, cb, fail) => { window._btSubscribeCb = cb; },
  write: (data, ok, fail) => {
    sentFrames.push(data);
    if (ok) ok();
  }
};

// 1. Cargar y ejecutar app.js
//
// N-75: antes solo se evaluaba app.js. El rewrite saco el gestor de cruces, el parser
// NMEA y el Courier a js/*.js, y jsdom no garantiza haberlos cargado a tiempo desde
// las etiquetas <script> de index.html: app.js reventaba con "SiteManager is not
// defined". Se cargan aqui EN EL MISMO ORDEN que los declara index.html, que es lo
// que hace el navegador de verdad.
const modulos = (htmlContent.match(/src="(js\/[^"]+\.js)"/g) || [])
  .map(m => m.replace(/src="|"/g, ''));
let initError = null;
try {
  modulos.forEach(rel => {
    const src = fs.readFileSync(path.join(__dirname, rel), 'utf8');
    // Un `const X = ...` dentro de eval() NO crea global -queda en el ambito del
    // propio eval-, mientras que dos <script> del navegador SI se ven entre si. Sin
    // esto app.js no encontraba SiteManager y el arnes acusaba a la app de un fallo
    // que solo existia en el arnes. Se exponen los nombres de primer nivel.
    const nombres = (src.match(/^(?:const|class|function|var|let)\s+([A-Za-z_$][\w$]*)/gm) || [])
      .map(m => m.split(/\s+/)[1]);
    const expone = nombres.map(n => `try{window.${n}=${n};}catch(e){}`).join('');
    window.eval(src + ';' + expone);
  });
  window.eval(jsContent);
} catch (e) {
  initError = e;
}
assert(!initError, `app.js se evalúa sin errores de sintaxis o inicialización (${initError ? initError.message : 'OK'})`);

// Disparar DOMContentLoaded
document.dispatchEvent(new window.Event('DOMContentLoaded'));
assert(true, 'Evento DOMContentLoaded ejecutado sin excepciones no controladas');

// 2. Probar cambio de Pestañas (Tab Switcher)
const navItems = document.querySelectorAll('.nav-item');
const tabPanes = document.querySelectorAll('.tab-content');
// N-75: la interfaz de 2 roles dejo 4 pestanas. tab-control desaparecio -sus mandos
// son la botonera tactica del operario- y tab-rtc tambien -el Courier vive en tab-diag-.
//
// SE CONSERVA, con el numero al dia (CLAUDE.md 8.quater): lo que mide -"la barra de
// abajo tiene exactamente las pestanas declaradas"- sigue valiendo; lo que ha cambiado
// es el sujeto, porque la V2 anade tab-depuracion. Son 5.
assert(navItems.length === 5, `Existen 5 botones de navegación inferior (detectados: ${navItems.length})`);

// Y el trinquete que faltaba, que es lo que impide que este numero vuelva a quedarse
// viejo: cada seccion tiene su boton y cada boton su seccion. Una pestana sin boton es
// una vista que existe y no se puede abrir -y no lo delataba nadie-; un boton sin
// seccion es un mando que no lleva a ninguna parte.
assert(navItems.length === tabPanes.length,
  `Tantos botones de navegación como secciones: ${navItems.length} botones / ${tabPanes.length} secciones`);
const sinBoton = [...tabPanes].filter(p => !document.querySelector(`.nav-item[data-tab="${p.id}"]`));
assert(sinBoton.length === 0,
  `Ninguna sección se queda sin botón que la abra (huérfanas: ${sinBoton.map(p => p.id).join(', ') || 'ninguna'})`);

const expectedTabs = ['tab-estado', 'tab-eventos', 'tab-tiempos', 'tab-diag', 'tab-depuracion'];
document.querySelectorAll('.admin-tab').forEach(t => { t.style.display = ''; });
expectedTabs.forEach(tabId => {
  const btn = document.querySelector(`.nav-item[data-tab="${tabId}"]`);
  assert(!!btn, `Botón de tab ${tabId} presente en el DOM`);
  btn.click();
  const targetPane = document.getElementById(tabId);
  assert(targetPane.classList.contains('active'), `Al hacer clic en [${tabId}], el contenedor recibe la clase .active`);
});

// 3. Probar Modal de Conexión Bluetooth
const btnDevice = document.getElementById('btnDevice');
btnDevice.click();
const btModal = document.getElementById('bt-modal');
assert(btModal.classList.contains('active'), 'Al pulsar [btnDevice] se abre el modal de Bluetooth (.active)');

// Simular clic en dispositivo Bluetooth
const firstBtItem = document.querySelector('.bt-device-item');
assert(!!firstBtItem, 'Dispositivos Bluetooth listados en el modal');
firstBtItem.click();
assert(!btModal.classList.contains('active'), 'Al conectar con un dispositivo, el modal se cierra automáticamente');
assert(btnDevice.classList.contains('connected'), 'El botón de dispositivo pasa a estado conectado (.connected)');

// 4. Inyectar telemetría NMEA en vivo ($STATUS)
assert(typeof window._btSubscribeCb === 'function', 'Callback de suscripción serie Bluetooth activo');

// SE REPARTE (CLAUDE.md 8.sexies). Esta trama llevaba `*5F`, que NO es su checksum -el
// real es `*04`-, y entraba igual porque parseNmeaTelemetry() tiraba el CRC sin mirarlo.
// Estas seis lineas decian DOS cosas a la vez: "la telemetria pinta los widgets" y, sin
// pretenderlo, "una trama con el checksum malo se pinta".
//
//   - la primera mitad SE CONSERVA aqui, con el checksum correcto: es lo que estas
//     lineas querian medir, y la seccion 6.bis ya dejaba escrito -antes de que nadie
//     validara nada- que "el dia que la app valide, estas tramas tienen que seguir
//     entrando";
//   - la segunda se MUDA a la seccion 9, donde se exige lo contrario y con su control:
//     una trama con el checksum malo NO se pinta, y deja rastro con su motivo.
//
// El checksum se escribe a mano AQUI Y SOLO AQUI, a proposito: es la unica trama del
// arnes cuyo CRC no sale de xorNmea(). Si se calculara tambien esta, un fallo del propio
// calculador la haria pasar igual, y no quedaria ni una linea que compare el XOR de la
// app contra un numero escrito por una persona.
const sampleTelemetry = '$STATUS,NODE:ESCLAVO,SERIE:SEM-E-01,MODO:AUTO,ESTADO:R1_V2,T:28,RF:95,RTT:75,BAT:12.8,HORA:14:30:00*04\n';
window._btSubscribeCb(sampleTelemetry);

const nodeNameEl = document.getElementById('node-name');
const cdNumEl = document.getElementById('cd-num');
const rfQualityEl = document.getElementById('rf-quality');
const batVoltageEl = document.getElementById('bat-voltage');
const rfRttElDom = document.getElementById('rf-rtt');
assert(nodeNameEl.textContent.includes('ESCLAVO'), `Telemetría actualiza rol del nodo: ${nodeNameEl.textContent}`);
assert(cdNumEl.textContent === '28', `Telemetría actualiza contador de segundos: ${cdNumEl.textContent}`);
assert(rfQualityEl.textContent === '95%', `Telemetría actualiza calidad RF: ${rfQualityEl.textContent}`);
assert(batVoltageEl.textContent.includes('12.8'), `Telemetría actualiza voltaje de batería: ${batVoltageEl.textContent}`);
// La cabecera de 2 roles no tiene reloj en vivo; la hora se guarda en el estado y la
// comprobacion de que NO se trunca por el split(':') la hace tests/test_unitarios.js.
assert(rfRttElDom.textContent === '75', `Telemetría actualiza el RTT del enlace: ${rfRttElDom.textContent}`);

// 4.1 Probar Gestor de Cruces (CRUD y Selección de Sitio)
const btnSelectSite = document.getElementById('btn-select-site');
btnSelectSite.click();
const siteModal = document.getElementById('site-modal');
assert(siteModal.classList.contains('active'), 'Al pulsar el selector de sitio se abre el modal de cruces');

// A. Crear un cruce nuevo
//
// La app de 2 roles pide el nombre por prompt() en vez de por un formulario modal.
// jsdom no implementa prompt(), asi que se sustituye: lo que se mide no es el dialogo
// sino que el cruce quede guardado, se active y aparezca en la lista.
let promptCola = [];
window.prompt = () => (promptCola.length ? promptCola.shift() : null);

const btnOpenAddSite = document.getElementById('btn-open-add-site');
assert(!!btnOpenAddSite, 'Boton de crear cruce presente en el modal de cruces');

promptCola = ['Obra Variante Km 45', 'PR 45+200 Calzada Derecha'];
btnOpenAddSite.click();

const currentSiteEl = document.getElementById('current-site-name');
assert(currentSiteEl.textContent.includes('Obra Variante Km 45'),
  `El nuevo cruce se activó en la cabecera: ${currentSiteEl.textContent}`);

// Y sobrevive a un re-render: si no persistiera, la lista lo perderia al reabrir.
btnSelectSite.click();
const listaEl = document.getElementById('dynamic-site-list');
assert(listaEl.textContent.includes('Obra Variante Km 45'),
  'El cruce creado persiste y vuelve a salir al reabrir el gestor');

// B. Editar y C. eliminar: la tarjeta vuelve a tener sus dos mandos, y llaman a las
// funciones de js/site_manager.js que se habian quedado sin llamador.
assert(!!document.querySelector('.btn-edit-site') && !!document.querySelector('.btn-delete-site'),
  'Las tarjetas de cruce exponen renombrar y eliminar');

promptCola = ['Obra Variante Km 45 [EDITADO]'];
document.querySelectorAll('.site-card').forEach(c => {
  if (c.textContent.includes('Obra Variante Km 45')) c.querySelector('.btn-edit-site').click();
});
btnSelectSite.click();
assert(listaEl.textContent.includes('[EDITADO]'),
  'El renombrado se ve en la lista sin recargar');

// D. Estres de escala: 20 cruces en el DOM
for (let i = 1; i <= 20; i++) {
  const pad = String(i).padStart(2, '0');
  promptCola = [`Cruce Corredor Km ${pad}`, `PR ${pad}+000 Doble Calzada`];
  btnOpenAddSite.click();
}

btnSelectSite.click();
const totalRenderedCards = listaEl.children.length;
assert(totalRenderedCards >= 20,
  `El DOM renderiza holgadamente los 20 cruces (total tarjetas: ${totalRenderedCards})`);

// Filtrado en vivo
const siteSearchInput = document.getElementById('site-search-input');
siteSearchInput.value = 'Km 15';
siteSearchInput.dispatchEvent(new window.Event('input', { bubbles: true }));
const filteredCards = listaEl.children.length;
assert(filteredCards === 1,
  `El buscador interactivo filtra instantáneamente entre los 20 cruces (encontrados: ${filteredCards})`);

// El poste (Maestro/Esclavo) ya no se elige en la tarjeta del cruce sino en el modal
// Bluetooth, donde cada modulo declara su data-node. La tarjeta selecciona el CRUCE.
const filteredEsclavoBtn = listaEl.querySelector('.site-card');
assert(!!filteredEsclavoBtn, 'El cruce filtrado se puede seleccionar desde su tarjeta');
if (filteredEsclavoBtn) filteredEsclavoBtn.click();
assert(currentSiteEl.textContent.includes('Km 15'), `Al seleccionar cruce filtrado, la app conmuta a Km 15: ${currentSiteEl.textContent}`);

// E. Probar Eliminación Masiva de 17 Cruces a través de los botones 🗑️ del DOM
btnSelectSite.click();
// Limpiar filtro de búsqueda
siteSearchInput.value = '';
siteSearchInput.dispatchEvent(new window.Event('input', { bubbles: true }));

const countBeforeDelete = document.querySelectorAll('.site-card').length;
for (let i = 0; i < 17; i++) {
  const delBtn = document.querySelector('.btn-delete-site');
  if (delBtn) delBtn.click();
}
const countAfterDelete = document.querySelectorAll('.site-card').length;
assert(countAfterDelete === countBeforeDelete - 17, `Eliminación de 17 cruces completada en el DOM (${countBeforeDelete} -> ${countAfterDelete})`);

// F. Agregar 4 nuevos cruces viales en el DOM
const nuevos4 = [
  { name: 'Cruce Túnel La Línea', loc: 'PR 50+100', p1: '👑 Maestro (Entrada)', p2: '📡 Esclavo (Salida)' },
  { name: 'Cruce Variante Guaduas', loc: 'PR 62+300', p1: '👑 Maestro (Norte)', p2: '📡 Esclavo (Sur)' },
  { name: 'Cruce Paso Urbano Villeta', loc: 'PR 75+800', p1: '👑 Maestro (P1)', p2: '📡 Esclavo (P2)' },
  { name: 'Cruce Puente Cundinamarca', loc: 'PR 88+000', p1: '👑 Maestro (P1)', p2: '📡 Esclavo (P2)' }
];

nuevos4.forEach(n => {
  promptCola = [n.name, n.loc];
  btnOpenAddSite.click();
});

btnSelectSite.click();
const finalCards = document.querySelectorAll('.site-card');
assert(finalCards.length === countAfterDelete + 4, `El DOM renderiza exactamente los ${countAfterDelete + 4} cruces finales`);

// G. Renombrar interactivamente todos los cruces en el DOM a través del botón ✏️
const editButtons = document.querySelectorAll('.btn-edit-site');
assert(editButtons.length === countAfterDelete + 4, `Todos los cruces poseen botón de edición ✏️ (${editButtons.length})`);

// Editar el primer cruce en el DOM
// N-75: el nombre de prueba baja de 41 a 30 caracteres. Lo que esta comprobacion mide
// es que el renombrado se vea al momento, no que quepan 41: SiteManager recorta a 32
// para que un nombre largo no parta la cabecera en dos lineas.
promptCola = ['Cruce Cundinamarca [RENOVADO]'];
editButtons[0].click();

btnSelectSite.click();
const updatedFirstCardTitle = document.querySelector('.site-card .site-card-title').textContent;
assert(updatedFirstCardTitle.includes('[RENOVADO]'), `La tarjeta del DOM cambió su nombre inmediatamente: ${updatedFirstCardTitle}`);

// Y el tope se aplica de verdad: un nombre que desbordaria la cabecera se recorta.
promptCola = ['Tramo Obra Kilometro 45 Variante Via al Llano Sector Norte'];
editButtons[0].click();
btnSelectSite.click();
const recortado = document.querySelector('.site-card .site-card-title').textContent;
assert(recortado.length <= 32,
  `Un nombre de 57 caracteres se recorta a ${recortado.length}: "${recortado}"`);

// H. Cerrar modal de cruces
const modalSiteClose = document.getElementById('modal-site-close');
modalSiteClose.click();
assert(!siteModal.classList.contains('active'), 'Modal de cruces se cierra con el botón ✕');

// 5. Inyectar Fuzzing de tramas corruptas
let fuzzError = null;
try {
  for (let i = 0; i < 50; i++) {
    window._btSubscribeCb(`$$$INVALID_TRASH_${i}_%&#\r\n`);
    window._btSubscribeCb(`$STATUS,CORRUPT_FIELD_${i}*00\n`);
    window._btSubscribeCb(null);
    window._btSubscribeCb('');
  }
} catch (e) {
  fuzzError = e;
}
assert(!fuzzError, `Fuzzing de 200 tramas corruptas descartadas limpiamente sin romper el hilo JS`);

// 6. Botonera tactica del operario y barrera de PIN
//
// Aqui esta la propiedad que se perdio en el rewrite y que este arnes no llego a
// medir porque reventaba antes: la app NO puede autorizarse sola. Solo el rojo de
// emergencia viaja sin PIN, y es deliberado (bluetooth.cpp:70-82).
document.querySelector('.nav-item[data-tab="tab-estado"]').click();

// N-83, INVERTIDA: esta comprobacion media la propiedad buena -la emergencia no pide
// PIN- sobre el literal equivocado. El equipo conectado aqui es un ESCLAVO, y en esa
// punta la caida segura NO es rojo fijo: es ambar intermitente con la talanquera
// ABIERTA. El comando pasa a llamarse como lo que hace. `FORZAR_ROJO` se queda en el
// Maestro, donde si pone rojo.
sentFrames = [];
const btnAmbarEmerg = document.getElementById('btn-op-ambar-emergencia');
assert(!!btnAmbarEmerg, 'Boton tactico de Ambar de Emergencia presente en el DOM');
btnAmbarEmerg.click();

const pinModal = document.getElementById('pin-modal');
assert(!pinModal.classList.contains('active'),
  'El Ambar de Emergencia NO pide PIN: se da desde el suelo, viendo el accidente');
assert(sentFrames.some(f => f.includes('CMD:AMBAR_EMERGENCIA') && !f.includes('PIN')),
  `El Ambar de Emergencia viaja SIN PIN, que es la forma que el firmware acepta: ${sentFrames.join(' | ')}`);

// Y la otra mitad, que antes NO se medía: el mando del Maestro contra un Esclavo no
// sale al canal. Dos puntas cuya caida segura es distinta no comparten boton, y si
// alguien las junta otra vez esta linea lo caza.
sentFrames = [];
const btnEmergency = document.getElementById('btn-op-emergency');
assert(!!btnEmergency, 'Boton tactico de Rojo Total presente en el DOM');
btnEmergency.click();
assert(sentFrames.length === 0,
  `El Rojo Total del Maestro NO se manda a un Esclavo: ${sentFrames.join(' | ')}`);

// -------------------------------------------------------------------------
// LAS TRES ORDENES DEL MAESTRO CONTRA UN ESCLAVO: NI SALEN NI PIDEN CLAVE
// -------------------------------------------------------------------------
// Estas comprobaciones son las de siempre INVERTIDAS (CLAUDE.md 8.quater). Hasta el
// 31/08 exigian que `MANUAL:CAMBIAR_TURNO` y los dos `SET_MODO` salieran al cable
// aqui, con este ESCLAVO delante -es el que declara el $STATUS de la seccion 4-. No
// era un descuido: cuando se escribieron, la app mandaba a ciegas y lo unico que se
// podia medir era el flujo del PIN. Median DOS propiedades a la vez -el enrutado por
// punta y la barrera de PIN- y solo ha cambiado la primera; la segunda esta viva y se
// ejerce entera unas lineas mas abajo, contra el MAESTRO, que es la punta que atiende
// esas tres ordenes (app.js: SOLO_MAESTRO).
//
// Lo que se exige ahora es lo que dice el firmware: el despachador del Esclavo no
// tiene esas ramas (Esclavo/src/bluetooth.cpp) y contestaria $ERR,CMD:DESCONOCIDO,
// que es el error que parece un boton roto.
//
// Y LO QUE SE MIDE NO ES SOLO "NO SALE": ES QUE NO SE PIDE LA CLAVE. La guarda de
// punta va DELANTE del PIN, y ese orden es la mitad util del arreglo. Si alguien la
// pone detras, el operario teclea cuatro digitos delante de un cruce parado para que
// la app le diga entonces que la orden no era para esta punta. Un teclado que aparece
// es la senal de que el orden se invirtio, asi que se mira el modal, no solo el cable.
sentFrames = [];
const btnStep = document.getElementById('btn-op-step');
assert(!!btnStep, 'Boton tactico de cambio de turno presente en el DOM');
btnStep.click();
assert(sentFrames.length === 0,
  `CAMBIAR_TURNO es del Maestro: contra un Esclavo NO sale al cable: ${sentFrames.join(' | ')}`);
assert(!pinModal.classList.contains('active'),
  'Contra un Esclavo NO se pide el PIN de una orden que no se va a mandar: la guarda de punta va DELANTE del teclado');

// Aqui vivia "con PIN erroneo no se autoriza nada", que tecleaba 9999 en el modal.
// INVERTIDA, y no a un segundo "el modal sigue cerrado" -esa linea no podria fallar
// sin que fallase la de arriba, y una comprobacion que no puede fallar sola es un
// adorno (CLAUDE.md 3.bis)-. Pasa a exigir la otra mitad del rechazo, que hasta hoy
// no medía nadie: QUE SE DIGA. Un boton que se queda mudo es indistinguible de un
// boton roto, y el operario lo pulsa otra vez. La app tiene que nombrar la punta que
// si atiende la orden. Esta linea tambien impide "arreglar" la guarda dejando el
// boton muerto.
const avisoPunta = document.querySelectorAll('.event-item')[0];
assert(!!avisoPunta && avisoPunta.textContent.includes('MANUAL:CAMBIAR_TURNO') &&
       avisoPunta.textContent.includes('MAESTRO'),
  `El rechazo por punta se explica y nombra quien SI atiende la orden: ${avisoPunta ? avisoPunta.textContent.trim() : '(sin evento)'}`);

// Los otros dos mandos de la botonera, con el mismo criterio y por separado: que uno
// consulte la punta no dice nada del de al lado -el hueco de N-83 fueron exactamente
// estos tres, y el resto de la app si preguntaba-.
['btn-op-auto', 'btn-op-amber'].forEach(id => {
  sentFrames = [];
  document.getElementById(id).click();
  assert(sentFrames.length === 0,
    `[${id}] SET_MODO es del Maestro: contra un Esclavo NO sale al cable: ${sentFrames.join(' | ')}`);
  assert(!pinModal.classList.contains('active'),
    `[${id}] tampoco abre el teclado de PIN: la guarda de punta va delante`);
});

// =========================================================================
// 6.bis LA MISMA BOTONERA CONTRA EL MAESTRO: LA GUARDA ENRUTA, NO TAPIA
// =========================================================================
// Aqui se conserva ENTERO el flujo del PIN, que es la propiedad buena que las seis
// comprobaciones de arriba median de paso y que se perderia si uno se limitara a
// invertirlas: "la primera orden que mueve luces pide el PIN", "un PIN erroneo no
// autoriza", "con cuatro digitos validos el modal se cierra y la orden sale" y "una
// vez verificado no se vuelve a pedir en cada pulsacion". Vuelven LITERALES, sobre
// los MISMOS tres botones que las median (CLAUDE.md 3.bis: no se reescribe logica ya
// probada), cambiando solo la punta que hay al otro lado.
//
// Que este bloque vaya DESPUES del anterior no es cosmetico: el flujo del PIN solo se
// puede medir con la clave sin verificar, y el bloque del Esclavo tiene que correr con
// la sesion virgen para poder afirmar que el teclado no se abrio. Invertir el orden
// dejaria las dos mitades sin medir.
//
// Y es ademas el control que le falta a la inversion: una guarda que no dejara pasar
// NADA tambien haria pasar las seis lineas de arriba. Lo que se exige es enrutado, no
// una tapia (CLAUDE.md 8.bis).
//
// Cambiar de punta es inyectar el $STATUS que la declara -exactamente lo que pasa en
// la calle al enlazar con el otro poste-, sin tocar ningun interno de la app. El
// checksum se CALCULA en vez de escribirse a mano: la trama de la seccion 4 lleva un
// *5F que no es el suyo -el real es *04- y la app la traga igual, porque
// parseNmeaTelemetry() parte por '*' y tira el checksum sin mirarlo, y
// NmeaParser.validarTrama() no tiene un solo llamador -ya medido en
// 01_Firmware/Simulaciones/simulador_puente_esp32.py:1295-. Escribir aqui un checksum
// falso seria acostumbrarse a que no importa; el dia que la app valide, estas dos
// tramas tienen que seguir entrando.
function xorNmea(payload) {
  let c = 0;
  for (const ch of payload) c ^= ch.charCodeAt(0);
  return c.toString(16).toUpperCase().padStart(2, '0');
}
function conectarComo(node, resto) {
  const payload = `STATUS,NODE:${node},${resto}`;
  window._btSubscribeCb(`$${payload}*${xorNmea(payload)}\n`);
}

conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:14:31:00');
// La inyeccion se verifica antes de apoyar nada en ella: un $STATUS que no entrara
// dejaria las diez comprobaciones siguientes midiendo un Esclavo y pasando por otra
// razon (CLAUDE.md 4).
assert(nodeNameEl.textContent.includes('MAESTRO'),
  `La app conmuta a MAESTRO con el $STATUS de la otra punta: ${nodeNameEl.textContent}`);

// La primera orden que MUEVE luces tiene que pedir el PIN.
sentFrames = [];
btnStep.click();
assert(sentFrames.length === 0,
  `Sin PIN verificado no sale NADA por el canal serie: ${sentFrames.join(' | ')}`);
assert(pinModal.classList.contains('active'),
  'La orden que mueve luces abre el teclado de PIN en vez de autorizarse sola');

// Un PIN equivocado no autoriza: el modal sigue abierto y no sale ninguna trama.
['9', '9', '9', '9'].forEach(d => {
  document.querySelector(`.pin-btn[data-key="${d}"]`).click();
});
assert(pinModal.classList.contains('active') && sentFrames.length === 0,
  `Con PIN erroneo no se autoriza nada: ${sentFrames.join(' | ')}`);

['1', '2', '3', '4'].forEach(d => {
  document.querySelector(`.pin-btn[data-key="${d}"]`).click();
});
assert(!pinModal.classList.contains('active'), 'Con 4 digitos validos el modal se cierra');
assert(sentFrames.some(f => f.includes('CMD:PIN:1234:MANUAL:CAMBIAR_TURNO')),
  `La orden sale con el PIN recien tecleado: ${sentFrames.join(' | ')}`);

// Y una vez verificado en la sesion no se vuelve a pedir en cada pulsacion: el
// operario esta en mitad de la calzada, no delante de un formulario.
['btn-op-auto', 'btn-op-amber'].forEach(id => {
  sentFrames = [];
  document.getElementById(id).click();
  assert(sentFrames.some(f => f.includes('CMD:PIN:1234:SET_MODO:')),
    `[${id}] con el PIN ya verificado la orden sale directa: ${sentFrames.join(' | ')}`);
  assert(!pinModal.classList.contains('active'),
    `[${id}] no vuelve a pedir el PIN dentro de la misma sesion`);
});

// Y las dos emergencias, ahora del reves. El par completo es lo que demuestra que el
// boton esta vivo y enrutado, no escondido: contra el Esclavo salia el ambar y se
// paraba el rojo; contra el Maestro tiene que ser exactamente al contrario.
sentFrames = [];
btnEmergency.click();
assert(sentFrames.some(f => f.includes('CMD:FORZAR_ROJO') && !f.includes('PIN')),
  `Contra el MAESTRO el Rojo Total SI sale, y sin PIN: una caida segura que pide clave no es una caida segura: ${sentFrames.join(' | ')}`);

sentFrames = [];
btnAmbarEmerg.click();
assert(sentFrames.length === 0,
  `El Ambar de Emergencia del Esclavo NO se manda a un Maestro -esa punta no conoce el literal-: ${sentFrames.join(' | ')}`);
// Y no solo se niega: no se ensena. El rotulo dice QUE maniobra y la punta decide
// CUAL de las dos esta en pantalla; ofrecer un boton que no se va a mandar es pedirle
// al operario que descubra a pulsaciones cual de los dos sirve.
assert(btnAmbarEmerg.style.display === 'none',
  `Contra un Maestro el mando de ambar ni siquiera se ofrece: display="${btnAmbarEmerg.style.display}"`);

// SOLICITAR_PASO es la simetrica: solo la entiende el Esclavo
// (Esclavo/src/bluetooth.cpp:128), asi que aqui NO puede salir.
sentFrames = [];
const btnSolicitar = document.querySelector('[data-cmd="SOLICITAR_PASO"]');
assert(!!btnSolicitar, 'El mando SOLICITAR_PASO (N-58) tiene boton en Modo Tecnico');
btnSolicitar.click();
assert(sentFrames.length === 0,
  `SOLICITAR_PASO es del Esclavo: contra un Maestro NO sale al cable: ${sentFrames.join(' | ')}`);

// =========================================================================
// 6.ter VUELTA AL ESCLAVO: LO QUE ESA PUNTA SI ATIENDE, Y EL PIN DE LA SESION
// =========================================================================
// Los mandos de tecnico declarados con data-cmd tambien salen por la misma puerta.
// Esta comprobacion es la de siempre y NO se toca -SE CONSERVA-: mide el despachador
// de data-cmd, que ya preguntaba por la punta antes del 31/08. Lo unico que cambia es
// que ahora se corre con el Esclavo delante a proposito y no por casualidad.
conectarComo('ESCLAVO', 'SERIE:SEM-E-01,MODO:AUTO,ESTADO:R1_V2,T:28,RF:95,RTT:75,BAT:12.8,HORA:14:32:00');
assert(nodeNameEl.textContent.includes('ESCLAVO'),
  `La app vuelve al ESCLAVO con su $STATUS: ${nodeNameEl.textContent}`);

sentFrames = [];
btnSolicitar.click();
assert(sentFrames.some(f => f.includes('SOLICITAR_PASO')),
  `SOLICITAR_PASO llega al canal serie: ${sentFrames.join(' | ')}`);
// El PIN es de la SESION y del operario, no del poste: cambiar de punta no vuelve a
// pedirlo. Si esto empezara a fallar, el tecnico tendria que reautorizarse cada vez
// que salta de un poste al otro, que es media obra.
assert(!pinModal.classList.contains('active'),
  'Cambiar de punta no vuelve a pedir el PIN: la autorizacion es del operario, no del equipo');

// 7. Probar Asistente Courier RTC
document.querySelector('.nav-item[data-tab="tab-diag"]').click();
const btnCourierCap = document.getElementById('btn-courier-capture');
const btnCourierInj = document.getElementById('btn-courier-inject');
assert(btnCourierInj.hasAttribute('disabled'), 'Botón Inyectar inicia deshabilitado');

btnCourierCap.click();
assert(!btnCourierInj.hasAttribute('disabled'), 'Al capturar tiempo en Maestro, botón Inyectar se habilita');

sentFrames = [];
btnCourierInj.click();
assert(sentFrames.some(f => f.includes('SET_RTC')), `Courier RTC generó comando SET_RTC compensado: ${sentFrames.join(' | ')}`);

// 8. Probar Ajustes de Tiempos (SET_TIEMPOS) y Rechazo del Firmware ($ERR)
document.querySelector('.nav-item[data-tab="tab-tiempos"]').click();
const inputVerde = document.getElementById('num-tiempo-verde');
const inputRojo = document.getElementById('num-tiempo-rojo');
const inputDespeje = document.getElementById('num-tiempo-despeje');
const btnAplicarTiempos = document.getElementById('btn-aplicar-tiempos');

assert(!!inputVerde && !!inputRojo && !!inputDespeje && !!btnAplicarTiempos, 'Formulario de Ajustes de Tiempos presente en el DOM');

// Asignar valores válidos: Verde=3 min, Rojo=4 min, Despeje=25 seg
inputVerde.value = '3';
inputRojo.value = '4';
inputDespeje.value = '25';

// INVERTIDA (CLAUDE.md 8.quater). Esta linea exigia que SET_TIEMPOS saliera al cable
// con el ESCLAVO enlazado -es la punta que dejo puesta la seccion 6.ter-. Median dos
// cosas a la vez y solo ha cambiado el enrutado: el formulario sigue componiendo bien
// los tres numeros, y eso se comprueba dos parrafos mas abajo contra el Maestro, que
// es quien tiene la rama (app.js: SOLO_MAESTRO).
//
// Aqui la propiedad nueva es ademas mas fina que en la botonera: la guarda se
// pregunta ANTES de validar los tres campos. Hacer rellenar bien un formulario cuya
// orden no se va a mandar es peor que negarla de entrada, asi que se entra con TRES
// VALORES VALIDOS -si la guarda estuviera detras del rango, un formulario invalido
// tambien daria cero tramas y esta linea pasaria sin medir nada-.
sentFrames = [];
btnAplicarTiempos.click();
assert(sentFrames.length === 0,
  `SET_TIEMPOS es del Maestro: con tres valores VALIDOS y un Esclavo delante no sale nada: ${sentFrames.join(' | ')}`);

// Y contra el Maestro sale: la mitad conservada de la comprobacion anterior, que es
// el control de que la guarda enruta en vez de tapiar el formulario.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:14:33:00');
assert(nodeNameEl.textContent.includes('MAESTRO'),
  `La app conmuta a MAESTRO para los tiempos de ciclo: ${nodeNameEl.textContent}`);

sentFrames = [];
btnAplicarTiempos.click();
assert(sentFrames.some(f => f.includes('SET_TIEMPOS:3,4,25')), `Comando SET_TIEMPOS generado correctamente: ${sentFrames.join(' | ')}`);

// Simular respuesta $ERR del firmware por valor fuera de rango.
// SE CONSERVA, con el checksum al dia: llevaba `*00` -que no es el suyo, el real es
// `*0F`- y entraba porque nadie validaba. Lo que esta linea mide es que un rechazo del
// firmware NO se oculta, y eso sigue en pie; lo que ha cambiado es que ahora la trama
// tiene que estar bien formada para llegar hasta ahi, igual que en la calle.
window._btSubscribeCb('$ERR,CMD:SET_TIEMPOS,DESC:RANGO*0F\n');
const eventItems = document.querySelectorAll('.event-item');
const lastEvent = eventItems[0];
assert(lastEvent && lastEvent.textContent.includes('Rechazo de Firmware: [SET_TIEMPOS] RANGO'), `La app muestra y registra el error de firmware $ERR,CMD:SET_TIEMPOS,DESC:RANGO sin ocultarlo`);

// =========================================================================
// 9. MODO DEPURACION: LAS TRAMAS EN CRUDO, LAS RECHAZADAS Y SU MOTIVO
// =========================================================================
// Aqui vive la mitad MUDADA de la seccion 4 (CLAUDE.md 8.sexies). Alli se exigia que
// una trama con el checksum malo se pintara -sin querer, pero se exigia-; aqui se
// exige lo contrario, y con el control que le falta a toda inversion: una guarda que
// no dejara pasar NADA cumpliria igual de bien "la mala no entra". Por eso cada
// rechazo se prueba junto a una trama BUENA que si tiene que entrar.
document.querySelector('.nav-item[data-tab="tab-depuracion"]').click();
const depuLista = document.getElementById('depu-lista');
const depuContadores = document.getElementById('depu-contadores');
const depuTexto = document.getElementById('depu-texto');
assert(!!depuLista && !!depuContadores && !!depuTexto,
  'La pantalla de depuración existe y está separada de la de operación');

// Vaciar la cinta para medir sobre una cuenta conocida -y de paso ejercer el boton-.
window.confirm = () => true;
document.getElementById('btn-depu-limpiar').click();
assert(window.RegistroCrudo.todas().length === 0,
  'El botón de vaciar deja la cinta a cero');
assert(depuLista.textContent.includes('Todavia no ha entrado ninguna trama'),
  'Con la cinta vacía la vista lo DECLARA en vez de enseñar una trama de ejemplo');
assert(!depuLista.textContent.includes('$STATUS') && !depuLista.textContent.includes('$ALARM'),
  'Y no hay ni una sola trama inventada en la vista vacía');

// --- La trama BUENA: el control de que esto no es una tapia ---
const cargaBuena = 'STATUS,NODE:ESCLAVO,SERIE:SEM-E-01,MODO:AUTO,ESTADO:R1_V2,T:44,RF:88,RTT:60,BAT:12.4,HORA:14:40:00';
window._btSubscribeCb(`$${cargaBuena}*${xorNmea(cargaBuena)}\n`);
assert(cdNumEl.textContent === '44',
  `Una trama con el checksum BUENO sigue entrando y pintando: cd-num=${cdNumEl.textContent}`);
assert(depuLista.textContent.includes(cargaBuena),
  'Y aparece en la cinta EN CRUDO, con sus campos tal y como llegaron');
assert(depuLista.textContent.includes('ACEPTADA'),
  'marcada como ACEPTADA, que es lo que la app hizo con ella');

// --- Rechazo 1: checksum malo. Es el defecto B1 de la V2, ya conectado. ---
const cargaMala = 'STATUS,NODE:MAESTRO,SERIE:SEM-M-99,MODO:AUTO,ESTADO:V1_R2,T:99,RF:11,RTT:900,BAT:9.9,HORA:03:41:00';
window._btSubscribeCb(`$${cargaMala}*00\n`);
assert(cdNumEl.textContent === '44',
  `Una trama con el CHECKSUM MALO no pinta nada: el contador sigue en el valor bueno (${cdNumEl.textContent}, no 99)`);
assert(!nodeNameEl.textContent.includes('MAESTRO'),
  `Ni cambia la punta que la app cree tener delante: ${nodeNameEl.textContent}`);
assert(depuLista.textContent.includes('RECHAZADA · CHECKSUM'),
  'La cinta la enseña como RECHAZADA nombrando el motivo: CHECKSUM');
assert(depuLista.textContent.includes(cargaMala),
  'Y guarda la trama EN CRUDO: sin la línea que no cuadró no hay nada que diagnosticar');
assert(depuLista.textContent.includes('esperado') || depuLista.textContent.includes('Checksum'),
  'con el detalle de la cuenta: qué CRC esperaba y cuál traía');

// --- Rechazo 2: sin forma de trama ---
window._btSubscribeCb('$SIN_ASTERISCO,NODE:MAESTRO\n');
assert(depuLista.textContent.includes('RECHAZADA · SIN_FORMA'),
  'Una línea sin el * del checksum se rechaza como SIN_FORMA');

// --- Rechazo 3: tipo desconocido, y con el checksum BUENO ---
// Es el que separa "no entiendo esta trama" de "esta trama viene rota": el CRC cuadra,
// asi que llego entera; lo que pasa es que esta app no tiene rama para ella. Antes se
// caia por el final del if/else sin dejar rastro de ninguna clase.
const cargaRara = 'PEPE,NODE:MAESTRO,ALGO:1';
window._btSubscribeCb(`$${cargaRara}*${xorNmea(cargaRara)}\n`);
assert(depuLista.textContent.includes('RECHAZADA · TIPO_DESCONOCIDO'),
  'Una trama con el checksum bueno y una cabecera que la app no lee se rechaza como TIPO_DESCONOCIDO');
assert(depuLista.textContent.includes('$PEPE'),
  'y también se guarda entera, que es como se descubre que el equipo emite algo nuevo');

// --- Los contadores de la ventana ---
const cuentaVista = window.RegistroCrudo.contadores();
assert(cuentaVista.total === 4 && cuentaVista.aceptadas === 1 && cuentaVista.rechazadas === 3,
  `Los contadores cuadran con lo inyectado: ${cuentaVista.total} tramas, ${cuentaVista.aceptadas} aceptadas, ${cuentaVista.rechazadas} rechazadas`);
assert(depuContadores.textContent.includes('3 rechazadas'),
  `La pantalla publica el recuento de la ventana: ${depuContadores.textContent}`);
assert(cuentaVista.porMotivo.CHECKSUM === 1 && cuentaVista.porMotivo.SIN_FORMA === 1 &&
       cuentaVista.porMotivo.TIPO_DESCONOCIDO === 1,
  'y desglosado por motivo, que es lo que distingue ruido de protocolo nuevo');

// --- El filtro NO borra: oculta, y lo dice ---
document.getElementById('btn-depu-rechazadas').click();
assert(!depuLista.textContent.includes(cargaBuena),
  'El filtro de "sólo rechazadas" deja fuera las aceptadas');
assert(depuLista.textContent.includes(cargaMala),
  'y conserva las rechazadas, que es lo que se está buscando');
assert(window.RegistroCrudo.todas().length === 4,
  'Filtrar no borra nada de la cinta: siguen las 4');
document.getElementById('btn-depu-todas').click();
assert(depuLista.textContent.includes(cargaBuena),
  'Volver a "todas" recupera las aceptadas');

// --- Un campo sin medida NO es un rechazo: entra y se declara ---
// Es la regla de RF_NO_MEDIDO llevada a la cinta. Si esto se contara como rechazo, la
// app diria que no pinto una trama que si pinto.
const cargaSinRf = 'STATUS,NODE:ESCLAVO,MODO:AUTO,ESTADO:R1_V2,T:12,RF:--,RTT:--,BAT:--,HORA:14:41:00';
window._btSubscribeCb(`$${cargaSinRf}*${xorNmea(cargaSinRf)}\n`);
assert(cdNumEl.textContent === '12',
  `Una trama con RF:-- SI entra y pinta lo que sí trae: cd-num=${cdNumEl.textContent}`);
assert(depuLista.textContent.includes('se pinto, pero'),
  'y la cinta anota el reparo en vez de tirarla: se pintó, pero el enlace no venía medido');
assert(window.RegistroCrudo.contadores().conReparos >= 1,
  'los contadores separan "aceptadas con reparo" de "rechazadas"');

// --- La bitácora del enlace RECIBE los rechazos: no hay un segundo registro al lado ---
function contarRechazosEnBitacora() {
  return window.RegistroEnlace.cargar().registros.filter(r => r.clase === 'RECHAZO').length;
}
const rechazosAntes = contarRechazosEnBitacora();
assert(rechazosAntes > 0,
  `Los rechazos se anotan en la bitácora del enlace, que es la que sobrevive al cierre de la app (${rechazosAntes} anotaciones)`);
const bitacoraTexto = window.RegistroEnlace.cargar().registros
  .filter(r => r.clase === 'RECHAZO').map(r => r.texto).join(' | ');
assert(bitacoraTexto.includes('Muestra:'),
  'y la anotación se lleva una muestra de la trama, porque la cinta se pierde al cerrar y ésta no');

// --- El estrangulador: mil tramas malas seguidas son UN suceso ---
// Sin esto, una radio ruidosa llenaria el tope de 400 de la bitacora con la misma linea
// repetida y se llevaria por delante la historia del enlace, que es justo lo que esa
// bitacora existe para guardar.
const antesDeLaRafaga = contarRechazosEnBitacora();
window._btSubscribeCb('$SIN_ASTERISCO_OTRA_VEZ\n');   // cambia el motivo: +1
for (let i = 0; i < 40; i++) {
  window._btSubscribeCb(`$STATUS,RAFAGA_${i}*00\n`);   // 40 seguidas del mismo motivo
}
const anotadasPorLaRafaga = contarRechazosEnBitacora() - antesDeLaRafaga;
assert(anotadasPorLaRafaga === 2,
  `41 rechazos seguidos dejan 2 anotaciones en la bitácora -una por cambio de motivo-, no 41 (dejaron ${anotadasPorLaRafaga})`);
assert(window.RegistroCrudo.todas().length === 46,
  `pero la cinta SÍ las tiene todas, una a una: ${window.RegistroCrudo.todas().length}`);
const conRafaga = window.RegistroEnlace.cargar().registros
  .filter(r => r.clase === 'RECHAZO').map(r => r.texto).join(' | ');
assert(/\d+ desde las \d{2}:\d{2}:\d{2}/.test(conRafaga),
  'y la anotación lleva la CUENTA de la racha con su hora de inicio, no una sola línea suelta');

// --- Lo que llega con la pestaña CERRADA no se pierde ---
// La vista no se repinta mientras nadie la mira -a un $STATUS por segundo serían
// sesenta filas de DOM por segundo gastando batería para nadie-. Esa optimización tiene
// una forma de fallar muy silenciosa: abrirse en blanco, diciendo "todavía no ha
// entrado ninguna trama" con la cinta llena. Que es justo la mentira que esta pantalla
// existe para no contar.
const cargaCerrada = 'STATUS,NODE:ESCLAVO,MODO:AUTO,ESTADO:R1_V2,T:77,RF:55,RTT:120,BAT:12.1,HORA:14:44:00';
document.querySelector('.nav-item[data-tab="tab-estado"]').click();
window._btSubscribeCb(`$${cargaCerrada}*${xorNmea(cargaCerrada)}\n`);
document.querySelector('.nav-item[data-tab="tab-depuracion"]').click();
assert(depuLista.textContent.includes(cargaCerrada),
  'Una trama que llega con la pestaña cerrada aparece al abrirla: la vista no se abre en blanco');
assert(depuContadores.textContent.includes('tramas'),
  `y los contadores se recomponen al abrir: ${depuContadores.textContent.slice(0, 60)}`);

// --- Sacar el registro del poste, sin internet ---
// Ninguna de las dos salidas llama a la red: el texto se compone en el telefono. La de
// abajo es ademas la que no puede fallar -dentro de un WebView una descarga puede no
// llegar a ninguna parte sin que la pagina se entere-.
document.getElementById('btn-depu-copiar').click();
assert(depuTexto.hidden === false && depuTexto.value.length > 0,
  'El botón de copiar deja el texto ENTERO a la vista para seleccionarlo (la salida que no puede fallar)');
assert(depuTexto.value.includes(cargaMala) && depuTexto.value.includes('CHECKSUM'),
  'El texto exportado lleva la trama rechazada en crudo y su motivo');
assert(depuTexto.value.includes(cargaBuena),
  'y también las aceptadas: sin ellas no se puede saber qué pasaba justo antes del fallo');
assert(depuTexto.value.includes('CONTADORES'),
  'con los contadores de la ventana en la cabecera');
assert(!/api\.whatsapp\.com/.test(depuTexto.value),
  'y se compone en el teléfono: en el cruce puede no haber internet');

// La descarga a fichero: jsdom no trae createObjectURL -no es cosa de la app-, asi que
// se le pone el que el navegador si tiene y se comprueba que el camino no revienta.
window.URL.createObjectURL = () => 'blob:prueba';
let errorExport = null;
try {
  document.getElementById('btn-depu-export').click();
} catch (e) {
  errorExport = e;
}
assert(!errorExport,
  `La descarga a fichero se ejecuta sin romper el hilo (${errorExport ? errorExport.message : 'OK'})`);

// Y el cierre de la propiedad: la vista de depuracion no escribe en la de operacion.
// (Una linea que no pueda fallar aqui seria un adorno, no una comprobacion: se mide el
// HTML de la otra pestana, que es donde apareceria el texto si se colara.)
assert(!document.getElementById('tab-estado').innerHTML.includes('RECHAZADA'),
  'La pantalla de operación no ha recibido ni una palabra de la de depuración');

console.log('='.repeat(80));
console.log(` RESULTADO JSDOM: ${testsPassed} PASS | ${testsFailed} FALLAS`);
console.log('='.repeat(80));

if (testsFailed > 0) {
  process.exit(1);
} else {
  process.exit(0);
}
