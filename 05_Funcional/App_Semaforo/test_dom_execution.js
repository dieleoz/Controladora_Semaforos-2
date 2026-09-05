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
// El segundo dialogo, del 04/09: lo que ABRE paso ya no pide el PIN, pide la via. Se
// declara aqui arriba porque a partir de este punto NINGUNA comprobacion de orden de
// barreras vale mirando solo el teclado: btn-op-step y btn-op-auto ya no lo abren, asi
// que un `!pinModal.active` sobre ellos pasaria a ser vacuamente cierto -verde midiendo
// nada, CLAUDE.md 3.bis-.
const viaModal = document.getElementById('via-modal');
assert(!!viaModal, 'El aviso de VIA DESPEJADA (04/09) existe en el DOM');
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
// Y LO QUE SE MIDE NO ES SOLO "NO SALE": ES QUE NO SE PIDE NADA AL OPERARIO. La guarda
// de punta va DELANTE de la que autoriza, y ese orden es la mitad util del arreglo. Si
// alguien la pone detras, el operario contesta delante de un cruce parado para que la
// app le diga entonces que la orden no era para esta punta. Un dialogo que aparece es
// la senal de que el orden se invirtio, asi que se miran los modales, no solo el cable.
//
// 04/09: se miran LOS DOS. Esta orden ya no abre el teclado -abre el aviso de via-, asi
// que mirar solo `pin-modal` dejaria de medir el orden y seguiria en verde: es el caso
// exacto de CLAUDE.md 8.sexies -"al quitar la guarda, la linea del RESULTADO no cae"-,
// y las lineas del ORDEN son las unicas que lo cazan. Y el coste de invertirlo aqui es
// peor que con el PIN: lo que se gasta pidiendo mirar el tramo para una orden que no se
// va a mandar no es tiempo, es la credibilidad de la pregunta.
sentFrames = [];
const btnStep = document.getElementById('btn-op-step');
assert(!!btnStep, 'Boton tactico de cambio de turno presente en el DOM');
btnStep.click();
assert(sentFrames.length === 0,
  `CAMBIAR_TURNO es del Maestro: contra un Esclavo NO sale al cable: ${sentFrames.join(' | ')}`);
assert(!pinModal.classList.contains('active'),
  'Contra un Esclavo NO se pide el PIN de una orden que no se va a mandar: la guarda de punta va DELANTE del teclado');
assert(!viaModal.classList.contains('active'),
  'ni se le pide que MIRE EL TRAMO para una orden que no se va a mandar: la guarda de punta va DELANTE del aviso de via');

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
// Los dos tienen detras barreras distintas -btn-op-auto pide SOLO la via; btn-op-amber
// pide el PIN y DESPUES la via (N-148)-, asi que se exigen los dos dialogos cerrados en
// los dos botones: el que le toca a cada uno mide su orden, y el otro impide que un
// cambio de guarda deje esta linea vacuamente verde.
['btn-op-auto', 'btn-op-amber'].forEach(id => {
  sentFrames = [];
  document.getElementById(id).click();
  assert(sentFrames.length === 0,
    `[${id}] SET_MODO es del Maestro: contra un Esclavo NO sale al cable: ${sentFrames.join(' | ')}`);
  assert(!pinModal.classList.contains('active'),
    `[${id}] tampoco abre el teclado de PIN: la guarda de punta va delante`);
  assert(!viaModal.classList.contains('active'),
    `[${id}] ni el aviso de via: la guarda de punta va delante de las dos`);
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
// 04/09 - SEGUNDO REPARTO, Y LA MITAD DEL PIN SE MUDA OTRA VEZ.
//
// El responsable decidio que el operario deje de teclear el PIN para lo que ABRE paso:
// lo sustituye "?Confirma que no quedan vehiculos en el tramo?". El motivo no es
// comodidad y hay que leerlo antes de tocar nada de aqui abajo: EL EQUIPO NO SABE SI
// QUEDAN VEHICULOS EN EL TRAMO, EL OPERARIO SI. Un PIN demuestra QUIEN eres, no que
// hayas MIRADO la via; y un banderillero que da paso cada tres minutos no teclea 1234
// cada vez -acaba escribiendo la clave en el capo con un rotulador, o dandosela a
// cualquiera-. `MANUAL:CAMBIAR_TURNO` y `SET_MODO:AUTO` pasan al aviso de via; el PIN
// se queda INTACTO para el tecnico: tiempos, modos, reloj, test de focos, y el AMBAR
// de esta misma botonera, que PARA en vez de abrir.
//
// Las once comprobaciones que cayeron median DOS cosas a la vez -"lo que mueve luces
// se autoriza antes de salir" y "asi es como funciona el teclado de PIN"-, y ninguno
// de los tres destinos de CLAUDE.md 8.quater vale para el conjunto: borrarlas se lleva
// el flujo del PIN, que no lo mide nadie mas en este arnes; invertirlas todas convierte
// nueve comprobaciones en repeticiones del mismo hecho. SE REPARTEN (8.sexies): la
// mitad de "que autorizacion falta" se invierte donde estaba, y la del PIN se muda con
// su BLOQUE LITERAL al mando de AMBAR, que sigue exigiendolo.
//
// Y EL ORDEN DE LOS DOS BLOQUES ES PARTE DE LA MEDIDA: el de la via va PRIMERO, con la
// sesion sin PIN. Asi lo que se demuestra no es "la via ademas del PIN" sino que LA VIA
// SOLA autoriza, que es la decision. Puesto al reves, el PIN de la sesion taparia la
// puerta nueva y el bloque de la via pasaria sin ejercerla.
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

// -------------------------------------------------------------------------
// LA ORDEN QUE ABRE PASO SE AUTORIZA MIRANDO LA VIA - INVERTIDA (04/09)
// -------------------------------------------------------------------------
// SE CONSERVA, y no es la misma linea que la de debajo: sigue exigiendo que sin
// autorizar NO salga un byte. Lo unico que ha cambiado es QUE autorizacion falta.
sentFrames = [];
btnStep.click();
assert(sentFrames.length === 0,
  `Sin confirmar la via no sale NADA por el canal serie: ${sentFrames.join(' | ')}`);

// INVERTIDA: hasta el 04/09 esta linea exigia `pinModal.active`. Y se exigen LAS DOS
// mitades porque la decision fue una SUSTITUCION, no un anadido. Si alguien deja el
// teclado delante del aviso, el operario teclea igual y la pregunta que de verdad
// importa se acaba contestando sin levantar la vista: dos barreras seguidas no suman,
// ensenan a decir que si.
assert(viaModal.classList.contains('active'),
  'La orden que ABRE paso abre el aviso de via en vez de autorizarse sola');
assert(!pinModal.classList.contains('active'),
  'y NO pide ademas el PIN: la via SUSTITUYE al teclado en lo que abre paso');
// El aviso nombra LA MANIOBRA, no "confirme la operacion". Un texto generico se
// contesta sin mirar, que es justo lo que la pregunta existe para impedir.
const viaManiobra = document.getElementById('via-maniobra');
assert(viaManiobra.textContent.includes('DAR PASO'),
  `El aviso dice que maniobra se va a hacer: "${viaManiobra.textContent.slice(0, 55)}..."`);

// -------------------------------------------------------------------------
// EL CONTROL POSITIVO DE LA INVERSION: TRAS CONFIRMAR, LA ORDEN SALE AL CABLE
// -------------------------------------------------------------------------
// CLAUDE.md 8.sexies, y es la parte que mas vale: una guarda que no dejara pasar NADA
// haria pasar las tres lineas de arriba igual de bien que la guarda correcta. Sin esto
// no se estaria midiendo la barrera nueva - se estaria midiendo una tapia.
//
// Y sale con el PIN DENTRO de la trama: el firmware no ha cambiado -sigue exigiendo
// CMD:PIN:1234:- y lo pone la app. Lo que cambio es a QUIEN se lo pide la app. Que la
// sesion NO tenga el PIN verificado en este punto es lo que convierte esta linea en la
// prueba de que la via SOLA autoriza.
document.getElementById('btn-via-confirmar').click();
assert(!viaModal.classList.contains('active'),
  'Al confirmar la via el aviso se cierra: quien confirma no se queda con el dialogo delante');
assert(sentFrames.some(f => f.includes('CMD:PIN:1234:MANUAL:CAMBIAR_TURNO')),
  `Tras confirmar la via la orden SALE al cable, con el PIN que pone la app: ${sentFrames.join(' | ')}`);

// -------------------------------------------------------------------------
// EL VALE DE VIA ES DE UNA ORDEN, NO DE LA SESION - INVERTIDA
// -------------------------------------------------------------------------
// Aqui vivia "[btn-op-auto] con el PIN ya verificado la orden sale directa". Ya no
// dice eso: haber mirado el tramo para DAR PASO no autoriza a arrancar el ciclo, que
// es otra maniobra y deja al equipo dando verdes solo sin volver a preguntar. Es la
// primera condicion de viaConfirmadaVigente() -`vale.orden !== orden`- y esta es la
// unica linea que la ejerce.
sentFrames = [];
document.getElementById('btn-op-auto').click();
assert(sentFrames.length === 0 && viaModal.classList.contains('active'),
  `[btn-op-auto] el vale de CAMBIAR_TURNO no arranca el ciclo: vuelve a preguntar por el tramo: ${sentFrames.join(' | ')}`);
assert(viaManiobra.textContent.includes('AUTOMATICO'),
  `y pregunta por SU maniobra, no repite la de la orden anterior: "${viaManiobra.textContent.slice(0, 55)}..."`);
document.getElementById('btn-via-confirmar').click();
assert(sentFrames.some(f => f.includes('CMD:PIN:1234:SET_MODO:AUTO')),
  `[btn-op-auto] con su propia via confirmada la orden SALE al cable: ${sentFrames.join(' | ')}`);

// =========================================================================
// 6.bis-2 EL FLUJO DEL PIN, MUDADO ENTERO AL MANDO QUE SIGUE PIDIENDOLO
// =========================================================================
// CLAUDE.md 8.sexies, cuarto destino: SE REPARTE. Las lineas de arriba afirmaban dos
// cosas, y solo una ha cambiado. La otra -"asi es como funciona el teclado de PIN":
// pide, rechaza el equivocado, acepta el bueno, dispara la orden que esperaba y no
// vuelve a pedir en la sesion- sigue viva palabra por palabra, porque el PIN no se ha
// retirado: se ha retirado de DOS ordenes.
//
// Se muda con su BLOQUE LITERAL (CLAUDE.md 3.bis: no se reescribe logica ya probada)
// al mando de AMBAR. No es un boton cualquiera: es de la MISMA botonera, va a la MISMA
// punta y conserva la guarda de PIN.
//
// Borrarlas se habria llevado por delante el flujo del PIN entero, que en este arnes
// no lo mide nadie mas: las cuatro puertas de mas abajo -SOLICITAR_PASO, el PIN de la
// sesion al cambiar de punta, el Courier y SET_TIEMPOS- se apoyan en que alguien lo
// tecleo AQUI. Eso es lo que las tiro a las once: no eran once defectos, era este
// bloque siendo el unico que autorizaba la sesion.
//
// 🔴 N-148 (05/09): AQUI HABIA UNA LINEA QUE CELEBRABA EL DEFECTO Y ADEMAS NO PODIA
// FALLAR. Decia `assert(!viaModal.active, 'y NO pregunta por la via: lo que para el
// trafico no necesita que nadie mire el tramo')`. Las dos mitades estaban mal:
//
//   - LA FRASE ERA FALSA. SET_MODO:AMBAR no para el trafico: modo_ambar_setup() pone
//     ambar intermitente en ESTA punta y manda CMD_GO_AMBAR a la otra, o sea que deja
//     entrar al corredor por los dos lados a la vez. Es la orden que MAS abre paso de
//     la botonera. Lo pidio el responsable el 05/09 y lo confirma el C++.
//   - Y LA LINEA NO PODIA FALLAR NI CON LA GUARDA PUESTA, que es peor: el PIN va
//     DELANTE de la via, asi que en ese instante -teclado abierto, PIN sin teclear- el
//     aviso de via no ha salido todavia ni tiene por que. Habria seguido en verde
//     despues del arreglo, midiendo nada (CLAUDE.md 3.bis).
//
// Se INVIERTE a lo unico que aqui distingue una app con guarda de una sin ella: que la
// via se pregunte DESPUES del PIN y ANTES de que salga un byte. La linea que mide el
// ORDEN es la que caza la regresion; la del resultado final no cae sola (CLAUDE.md
// 8.sexies).
const btnAmber = document.getElementById('btn-op-amber');
sentFrames = [];
btnAmber.click();
assert(sentFrames.length === 0,
  `Sin PIN verificado no sale NADA por el canal serie: ${sentFrames.join(' | ')}`);
assert(pinModal.classList.contains('active'),
  'AMBAR sigue abriendo el teclado de PIN en vez de autorizarse solo');
assert(!viaModal.classList.contains('active'),
  'y el aviso de via NO se adelanta al teclado: sellar "he mirado" y meter luego cuatro digitos con guantes deja el tramo sin vigilar mientras se teclea');

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
// INVERTIDA (N-148). Aqui se exigia que la orden saliera con el PIN recien tecleado. Ya
// no sale: el PIN dice QUIEN eres y esta orden abre el corredor por los dos lados, asi
// que detras del teclado queda la pregunta por el tramo. Lo que se conserva entero es
// que sin ella NO SALE UN BYTE.
assert(sentFrames.length === 0 && viaModal.classList.contains('active'),
  `Tecleado el PIN, AMBAR pregunta por el tramo y todavia no manda nada: ${sentFrames.join(' | ')}`);
assert(viaManiobra.textContent.includes('AMBAR') && viaManiobra.textContent.includes('dos'),
  `y el aviso dice QUE maniobra y que abre las DOS puntas, no "confirme la operacion": "${viaManiobra.textContent.slice(0, 70)}..."`);
// EL CONTROL POSITIVO DE LA INVERSION (CLAUDE.md 8.sexies): sin esto, una guarda que no
// dejara pasar nada aprobaria las dos lineas de arriba igual de bien. Y es ademas donde
// se comprueba que el PIN de la sesion sigue viajando entero en la trama.
document.getElementById('btn-via-confirmar').click();
assert(sentFrames.some(f => f.includes('CMD:PIN:1234:SET_MODO:AMBAR')),
  `Confirmado el tramo, la orden sale con el PIN que se acaba de teclear: ${sentFrames.join(' | ')}`);

// Y una vez verificado en la sesion no se vuelve a pedir en cada pulsacion: el
// operario esta en mitad de la calzada, no delante de un formulario. El vale de via
// tampoco, mientras sea la MISMA orden, dentro de los 30 s y sin que la fase haya
// cambiado: es la regla 3 de 3.ter, y esta es la unica linea que la ejerce en verde.
sentFrames = [];
btnAmber.click();
assert(sentFrames.some(f => f.includes('CMD:PIN:1234:SET_MODO:AMBAR')),
  `[btn-op-amber] con el PIN y el tramo ya confirmados la orden sale directa: ${sentFrames.join(' | ')}`);
assert(!pinModal.classList.contains('active') && !viaModal.classList.contains('active'),
  '[btn-op-amber] no vuelve a pedir ni el PIN ni el tramo dentro de la misma sesion y la misma fase');

// -------------------------------------------------------------------------
// LA MITAD QUE NO EXISTIA ANTES DEL 04/09: EL PIN NO SUPLE A LA VIA
// -------------------------------------------------------------------------
// Es la comprobacion que sostiene la decision entera. Si un PIN verificado sirviera
// para lo que ABRE paso, la pregunta por el tramo seria un tramite que se salta solo
// con haber tecleado antes, y el operario volveria a dar paso sin mirar.
//
// La fase cambia -que es exactamente lo que pasa en la calle en cuanto el equipo
// obedece un CAMBIAR_TURNO-, asi que el vale de mas arriba deja de valer: es la tercera
// condicion de viaConfirmadaVigente(). Se hace inyectando el $STATUS que lo declara, no
// tocando internos de la app.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:MANUAL,ESTADO:R1_V2,T:31,RF:97,RTT:70,BAT:12.9,HORA:14:31:30');
sentFrames = [];
btnStep.click();
assert(sentFrames.length === 0 && viaModal.classList.contains('active'),
  `Con el PIN de la sesion verificado y la fase cambiada, la orden que abre paso VUELVE a pedir la via: ${sentFrames.join(' | ')}`);
// Y "Todavia no" no manda nada, que es la otra mitad de una pregunta de verdad: si la
// respuesta negativa no parase la orden, preguntar seria teatro.
document.getElementById('btn-via-cancelar').click();
assert(sentFrames.length === 0 && !viaModal.classList.contains('active'),
  `Contestar "todavia no" cierra el aviso y NO manda la orden: ${sentFrames.join(' | ')}`);

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

// =========================================================================
// 6.quater B2: EL TECLADO CERRADO NO ARMA LA SESION, Y CERRARLO CANCELA
// =========================================================================
// Los diez botones del teclado siguen en el arbol con el modal cerrado -solo estan
// ocultos por CSS-, y sus manejadores no preguntaban por el modal: cualquier click
// sobre ellos llamaba a validatePin() y armaba state.pinVerificado. Es el estado de
// una barrera puesto a CIERTO sin que la barrera se haya abierto.
//
// Y no es teorico: en N-83 el arnes tecleaba 1234 sobre un modal que la guarda de
// punta nunca abrio, la sesion se autorizaba sola, y SOLICITAR_PASO seguia dando [OK]
// mientras seis comprobaciones de al lado caian. El defecto no se cobro en la calle:
// se cobro quitandole capacidad de detectar a otra prueba.
//
// ESTE BLOQUE MONTA UNA APP NUEVA, Y ESA NECESIDAD ES EL SEGUNDO HALLAZGO. La sesion
// de arriba ya tiene el PIN verificado y NO HAY FORMA DE VOLVER ATRAS: pinVerificado
// se pone a true en una sola linea y no se apaga en ninguna -ni al cerrar el teclado,
// ni al cambiar de punta, ni al caerse el enlace, ni con el tiempo-. Para medir una
// sesion sin autorizar hace falta un navegador nuevo, que en la calle significa cerrar
// la app. Cuanto tiene que durar esa autorizacion lo decide el responsable; lo que el
// arnes deja escrito es que hoy dura lo que dure el proceso.
function montarAppLimpia() {
  const dom2 = new JSDOM(htmlContent, { runScripts: 'dangerously', resources: 'usable', url: 'http://localhost/' });
  const w = dom2.window;
  const almacen = {};
  w.localStorage = {
    getItem: (k) => almacen[k] || null,
    setItem: (k, v) => { almacen[k] = String(v); },
    removeItem: (k) => { delete almacen[k]; },
    clear: () => { Object.keys(almacen).forEach(k => delete almacen[k]); }
  };
  w.navigator.vibrate = () => true;
  const tramas = [];
  w.bluetoothSerial = {
    isEnabled: (ok) => ok(),
    list: (ok) => ok([{ name: 'JDY-31 Maestro', id: '00:11:22:33:44:55', address: '00:11:22:33:44:55' }]),
    connect: (mac, ok) => ok(),
    disconnect: (ok) => ok && ok(),
    subscribe: (delim, cb) => { w._btSubscribeCb = cb; },
    write: (data, ok) => { tramas.push(data); if (ok) ok(); }
  };
  modulos.forEach(rel => {
    const src = fs.readFileSync(path.join(__dirname, rel), 'utf8');
    const nombres = (src.match(/^(?:const|class|function|var|let)\s+([A-Za-z_$][\w$]*)/gm) || [])
      .map(m => m.split(/\s+/)[1]);
    w.eval(src + ';' + nombres.map(n => `try{window.${n}=${n};}catch(e){}`).join(''));
  });
  w.eval(jsContent);
  w.document.dispatchEvent(new w.Event('DOMContentLoaded'));
  w.document.getElementById('btnDevice').click();
  w.document.querySelector('.bt-device-item').click();
  const carga = 'STATUS,NODE:MAESTRO,SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:14:31:00';
  w._btSubscribeCb(`$${carga}*${xorNmea(carga)}\n`);
  return { d: w.document, tramas };
}

const limpia = montarAppLimpia();
const pinModal2 = limpia.d.getElementById('pin-modal');
const pinD1 = limpia.d.getElementById('pin-d1');

// (0) LA GUARDA DEL EMISOR, QUE NO MEDIA NADIE.
//
// Salio del censo de este encargo: de los catorce llamadores de
// enviarComandoFirmware() hay CUATRO -SET_MODO:DEGRADADO, SET_TIEMPOS y los dos
// SET_RTC- que no preguntan por state.pinVerificado antes de llamar. Para esos, el
// `if` de dentro del emisor no es un refuerzo: es la unica barrera que tienen.
//
// Y estaba sin vigilar. Al quitarlo del fuente, los 38 packs y este arnes seguian en
// verde, porque las tres puertas que si se median tienen su propia guarda encima y
// porque cuando el arnes llega al Courier ya ha tecleado el PIN. Es la barrera de
// abajo tapando la de arriba de CLAUDE.md 8.sexies, vista desde el otro lado: aqui
// la de arriba tapaba que nadie miraba la de abajo.
//
// Se mide por SET_TIEMPOS, que es el llamador sin guarda propia mas facil de ejercer.
limpia.d.getElementById('num-tiempo-verde').value = '5';
limpia.d.getElementById('num-tiempo-rojo').value = '5';
limpia.d.getElementById('num-tiempo-despeje').value = '30';
limpia.tramas.length = 0;
limpia.d.getElementById('btn-aplicar-tiempos').click();
assert(limpia.tramas.length === 0,
  `SET_TIEMPOS no tiene guarda propia: sin PIN lo para el emisor, y esto lo comprueba: ${limpia.tramas.join(' | ')}`);

// (1) TECLADO FANTASMA: se teclea el PIN BUENO sobre un modal que nadie abrio.
limpia.tramas.length = 0;
['1', '2', '3', '4'].forEach(dg => limpia.d.querySelector(`.pin-btn[data-key="${dg}"]`).click());
limpia.d.getElementById('btn-pin-ok').click();
// El teclado no ha recibido nada: los puntos son lo que ve el operario, y son tambien
// la prueba de que la pulsacion se paro EN LA ENTRADA y no solo al autorizar.
assert(!pinD1.classList.contains('filled'),
  'Con el teclado cerrado las pulsaciones no entran ni en el buffer: el primer punto sigue vacio');
// Y la sesion NO quedo autorizada, que se mide como se mide en la calle: pidiendo algo.
//
// EL VEHICULO SE MUDA A `btn-op-amber` (04/09), y no es cosmetico. Con `btn-op-step`,
// que desde hoy se autoriza mirando la via, un `tramas.length === 0` seguiria saliendo
// VERDE por el motivo equivocado: nadie confirmo el tramo. La linea pasaria a no decir
// nada del PIN -que es lo unico que este bloque existe para medir- y el teclado fantasma
// podria volver sin que este arnes se enterara. Es CLAUDE.md 8.sexies exacto: la barrera
// de abajo tapando a la de arriba. AMBAR es de la misma botonera y conserva el PIN, asi
// que el bloque se muda LITERAL y vuelve a medir lo suyo.
limpia.d.getElementById('btn-op-amber').click();
assert(limpia.tramas.length === 0,
  `Tras teclear el PIN con el modal cerrado la sesion sigue SIN autorizar: ${limpia.tramas.join(' | ')}`);
assert(pinModal2.classList.contains('active'),
  'y la orden que mueve luces abre el teclado en vez de salir con una autorizacion que nadie dio');

// (2) CERRAR EL TECLADO CANCELA LA ORDEN QUE ESPERABA DETRAS.
// pedirPin() dejo SET_MODO:AMBAR en cola. El operario se arrepiente y cierra con la X.
limpia.d.getElementById('modal-pin-close').click();
// Mas tarde alguien teclea el PIN por OTRO motivo: subir a Tecnico desde la cabecera.
limpia.d.getElementById('btn-toggle-role').click();
assert(pinModal2.classList.contains('active'),
  'El interruptor de rol abre el teclado: es la otra puerta legitima, y sin ella esto no mide nada');
limpia.tramas.length = 0;
['1', '2', '3', '4'].forEach(dg => limpia.d.querySelector(`.pin-btn[data-key="${dg}"]`).click());
assert(limpia.tramas.length === 0,
  `La orden cancelada NO se dispara con la siguiente clave, que se tecleo para otra cosa: ${limpia.tramas.join(' | ')}`);
// El control positivo, sin el cual lo de arriba lo aprobaria tambien una app que no
// hiciera nada nunca: esta autorizacion SI hace lo suyo (CLAUDE.md 8.sexies).
assert(limpia.d.getElementById('role-label').textContent === 'Técnico',
  `y la clave hace LO QUE SE TECLEO: sube a Tecnico (rol: ${limpia.d.getElementById('role-label').textContent})`);
// Y los cuatro digitos buenos no se quedan en memoria detras del modal cerrado: son
// los que un OK suelto reutilizaria el dia que la autorizacion caduque.
assert(!pinD1.classList.contains('filled'),
  'Una validacion correcta vacia el teclado: el PIN bueno no queda guardado tras el modal');

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
// 8.bis N-150: LOS TIEMPOS ENTRAN Y EL CRUCE SE QUEDA EN ROJO
// =========================================================================
// Reportado el 05/09 con el equipo delante: "se le da aplicar tiempos y se queda en
// rojo maestro y esclavo y no cambian". El defecto no era del firmware -el menu deja
// el cruce en rojo A PROPOSITO, con CMD_GO_RED cada 3 s- sino que el ultimo paso es de
// una persona y la app no lo decia ni lo ofrecia.
//
// LO QUE SE MIDE AQUI ES EL ENCADENADO, NO EL CARTEL. Que exista un div se ve leyendo
// el HTML; lo que hay que ejercer es que ese cartel dependa de LO QUE VOLVIO y no de lo
// que se mando, y que el mando que ofrece siga pasando por el aviso de via. Las cuatro
// lineas de abajo caen todas si alguien "simplifica" colgandolo del submit.
const avisoParado = document.getElementById('aviso-tiempos-parado');
const btnArrancar = document.getElementById('btn-arrancar-ciclo');
assert(!!avisoParado && !!btnArrancar,
  'N-150: el aviso de "el cruce sigue parado" y su mando existen en el DOM');

// LA SITUACION REAL DEL REPORTE, puesta con una trama y no tocando internos: el
// operario ya saco el equipo al MENU -es lo que hay que hacer para que acepte los
// tiempos- y el cruce esta en ROJO en las dos puntas. De paso caduca el vale de via
// que la seccion 5 dejo vivo para SET_MODO:AUTO: el vale sella la FASE, y la fase
// acaba de cambiar. Sin esto el bloque 4 mediria la vigencia del vale en vez de la
// barrera, y pasaria por el motivo equivocado.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:MENU,ESTADO:R1_R2,T:--,RF:97,RTT:70,BAT:12.9,HORA:14:33:30');

// 1. EL CONTROL, Y VA PRIMERO. En este punto la app YA mando un SET_TIEMPOS al cable
//    -cinco lineas mas arriba- y lo unico que ha vuelto es un $ERR de RANGO. Si el
//    cartel estuviera colgado del envio, ya estaria abierto: estaria ofreciendo
//    arrancar el ciclo con los tiempos VIEJOS mientras el operario cree que son los
//    que acaba de teclear. Sin esta linea, la de abajo pasaria igual con el defecto
//    dentro (CLAUDE.md 8.sexies: una comprobacion que no puede fallar sola es adorno).
assert(avisoParado.hidden === true,
  'N-150: tras MANDAR los tiempos y recibir un $ERR, el cartel sigue cerrado: cuelga del acuse, no del envio');

// 2. Y con el $ACK del equipo se abre.
const ackTiemposOk = 'ACK,CMD:SET_TIEMPOS,RESULT:OK';
const errEnMarcha = 'ERR,CMD:SET_TIEMPOS,DESC:EN_MARCHA_PARE_EL_MODO';
window._btSubscribeCb(`$${ackTiemposOk}*${xorNmea(ackTiemposOk)}\n`);
assert(avisoParado.hidden === false,
  'N-150: con $ACK,CMD:SET_TIEMPOS,RESULT:OK el cartel se abre y explica que el cruce sigue en rojo');

// 3. Y el acuse NO se pinta con el generico "ACEPTADA" en verde. Es la trampa que
//    ACK_TEXTO existe para evitar: un si a secas sobre algo que todavia no ha pasado.
const eventoTiempos = document.querySelectorAll('.event-item')[0];
assert(eventoTiempos && eventoTiempos.textContent.includes('TIEMPOS GUARDADOS') &&
       eventoTiempos.textContent.includes('ROJO'),
  `N-150: el acuse de SET_TIEMPOS dice que el cruce sigue parado, no solo "ACEPTADA": "${eventoTiempos ? eventoTiempos.textContent.slice(0, 70) : '(sin evento)'}..."`);

// 4. EL MANDO NO ARRANCA NADA SOLO. Arrancar el Automatico ABRE PASO, asi que tiene
//    que pasar por el aviso de via igual que el boton de la botonera (CLAUDE.md 6).
const btnViaConfirmarN150 = document.getElementById('btn-via-confirmar');
sentFrames = [];
btnArrancar.click();
assert(sentFrames.length === 0,
  `N-150: el mando de arrancar NO manda nada por si solo: ${sentFrames.join(' | ')}`);
assert(viaModal.classList.contains('active'),
  'N-150: el mando de arrancar pasa por el aviso de via, porque la orden ABRE paso');
assert(document.getElementById('via-maniobra').textContent.includes('AUTOMATICO'),
  'N-150: y el aviso nombra la maniobra que se va a hacer (AUTOMATICO), no "confirme"');

// 5. Confirmada la via, sale la MISMA orden que la botonera. Un literal propio aqui
//    seria una segunda puerta de salida para SET_MODO:AUTO.
sentFrames = [];
btnViaConfirmarN150.click();
assert(sentFrames.some(f => f.includes('SET_MODO:AUTO')),
  `N-150: confirmada la via, arranca el ciclo con SET_MODO:AUTO: ${sentFrames.join(' | ')}`);

// 6. Y EL CARTEL SE RETIRA CUANDO EL EQUIPO DICE QUE CICLA, no cuando la app manda.
//    Mientras el $STATUS no traiga MODO:AUTO el cartel sigue puesto, que es lo correcto:
//    la orden pudo perderse en la radio despues de salir.
assert(avisoParado.hidden === false,
  'N-150: mandada la orden, el cartel SIGUE puesto: mandar no es que el equipo cicle');
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:V1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:14:34:00');
assert(avisoParado.hidden === true,
  'N-150: y se retira cuando el equipo declara MODO:AUTO en su $STATUS');

// 7. El rechazo por ciclo en marcha dice los TRES pasos, y el tercero es el que faltaba.
window._btSubscribeCb(`$${errEnMarcha}*${xorNmea(errEnMarcha)}\n`);
const eventoEnMarcha = document.querySelectorAll('.event-item')[0];
assert(eventoEnMarcha && eventoEnMarcha.textContent.includes('VOLVER AL MENU') &&
       eventoEnMarcha.textContent.includes('ARRANQUE EL CICLO'),
  `N-150: EN_MARCHA_PARE_EL_MODO nombra los tres pasos, arranque incluido: "${eventoEnMarcha ? eventoEnMarcha.textContent.slice(0, 70) : '(sin evento)'}..."`);

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


// =========================================================================
// 10. EL DIARIO DE ORDENES: LA TERNA ORDEN / RESPUESTA / EFECTO
// =========================================================================
// POR QUE VIVE AQUI Y NO EN UN SCRATCHPAD (CLAUDE.md 3). El diario nacio el 04/09 de
// una perdida real de banco -el Courier devolvia "formato invalido", se exportaron 300
// tramas y LA ORDEN QUE SE MANDO NO ESTABA EN NINGUNA-, y los trece chequeos que lo
// ejercian se escribieron en un fichero de sesion que se borra al cerrarla. Un
// instrumento que no esta en la compuerta no mide nada Y NO DEJA RASTRO DE QUE FALTA:
// manana alguien refactoriza la rama del $ACK y el diario deja de correlacionar en
// silencio, con los 142 verdes de arriba intactos. Se absorben aqui, que es el unico
// arnes que EJECUTA la app y que la compuerta cuenta.
//
// Las siete propiedades que se miden, en el orden en que se leen tres lineas del
// diario: que la orden se anota tal cual salio, que la respuesta se pega a SU orden,
// que el unico alias del firmware se respeta, que un rechazo que no nombra la orden no
// se reparte a ojo, que el efecto no afirma sobre datos que no tiene, que el PIN sale
// tapado por los dos lados y entero al cable, y que una orden que la app freno entra en
// el diario pero NO en la cinta.
document.querySelector('.nav-item[data-tab="tab-depuracion"]').click();
const diarioLista = document.getElementById('diario-lista');
const diarioTexto = document.getElementById('diario-texto');
const diarioResumen = document.getElementById('diario-resumen');
assert(!!diarioLista && !!diarioTexto && !!diarioResumen,
  'El diario de ordenes tiene su propia vista, separada de la cinta en crudo');

// Los dos registros a cero, con los botones de la app -que de paso quedan ejercidos-.
// Vaciar el diario borra tambien el ultimo $STATUS visto, que es lo que hace falta para
// que el bloque del efecto empiece sin un "antes" heredado de la seccion anterior.
window.confirm = () => true;
document.getElementById('btn-depu-limpiar').click();
document.getElementById('btn-diario-limpiar').click();
assert(window.DiarioOrdenes.todas().length === 0 && window.RegistroCrudo.todas().length === 0,
  'El boton de vaciar el diario lo deja a cero, igual que el de la cinta');

const D = window.DiarioOrdenes;
const R = window.RegistroCrudo;

// N-148: AMBAR PREGUNTA POR EL TRAMO, ASI QUE EN ESTA SECCION HAY QUE CONTESTARLE.
//
// Lo que se mide de aqui abajo es EL DIARIO -que una orden que sale se anota, que el
// $ACK se le pega, que el efecto no afirma sobre lo que no vio-, no la guarda. La
// guarda se ejerce entera en 6.bis-2, con sus dos mitades y su control positivo.
//
// El `if` no es tolerancia: el vale de via dura 30 s y vale para la MISMA orden y la
// MISMA fase, asi que la primera pulsacion tras cada `conectarComo` que cambie de fase
// abre el aviso y las siguientes no. Un helper que clicara el confirmar a ciegas
// reventaria en las segundas -no hay dialogo que cerrar- y uno que no lo clicara nunca
// dejaria media seccion midiendo ordenes que no salieron. Lo que NO hace este helper es
// tragarse el caso de "no salio nada": eso lo siguen exigiendo las lineas de abajo, una
// por una, sobre las tramas de verdad.
function darAmbar() {
  btnAmber.click();
  if (viaModal.classList.contains('active')) {
    document.getElementById('btn-via-confirmar').click();
  }
}

// La punta vuelve a ser el MAESTRO: la seccion 9 dejo un ESCLAVO delante y SET_MODO
// esta en SOLO_MAESTRO, asi que sin esto los botones avisarian de la otra punta y no
// saldria una sola orden -y las lineas de abajo pasarian por la razon equivocada-.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:MANUAL,ESTADO:R1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:15:00:00');
assert(nodeNameEl.textContent.includes('MAESTRO'),
  `La app tiene delante al MAESTRO antes de dar ninguna orden: ${nodeNameEl.textContent}`);

// -------------------------------------------------------------------------
// 10.1 UNA ORDEN QUE SALE SE ANOTA CON VEREDICTO ENVIADA, Y TAL CUAL SALIO
// -------------------------------------------------------------------------
// Es la mitad que faltaba en la cinta hasta el 04/09: grababa lo que ENTRA. Y se exige
// la trama LITERAL, no un resumen: cuando se inyecto el defecto en la guarda de via, la
// linea de "no sale ningun byte" NO cayo -otra barrera mas abajo frenaba igual- y lo
// unico que delato el fallo fue el CONTENIDO de la trama que si salio.
sentFrames = [];
darAmbar();
const enviadas = R.todas().filter(x => x.veredicto === R.ENVIADA);
assert(enviadas.length === 1,
  `Una orden que sale deja UNA anotacion con veredicto ENVIADA en la cinta (${enviadas.length})`);
assert(enviadas.length === 1 && /^CMD:PIN:\*{4}:SET_MODO:AMBAR\r\n$/.test(enviadas[0].linea),
  `y es la trama TAL CUAL se escribio, con su CR LF y sin reconstruir: ${JSON.stringify(enviadas[0] && enviadas[0].linea)}`);
const ordenAmbar = D.todas().filter(x => x.clase === 'ORDEN').pop();
assert(!!ordenAmbar && ordenAmbar.orden === 'SET_MODO:AMBAR' && ordenAmbar.salio === true,
  `y la MISMA orden abre su entrada en el diario: ${ordenAmbar ? ordenAmbar.orden : '(ninguna)'}`);
assert(!!ordenAmbar && ordenAmbar.linea === (enviadas[0] && enviadas[0].linea),
  'con la misma linea literal en los dos registros: uno no es el resumen del otro');

// -------------------------------------------------------------------------
// 10.2 EL PIN SALE TAPADO POR LOS DOS LADOS, Y ENTERO AL CABLE
// -------------------------------------------------------------------------
// Lo que se exporta se manda por WhatsApp: es el uso real, no una hipotesis. Se tapa AL
// ANOTAR y no al pintar, asi que hay que comprobarlo en los CUATRO sitios por los que
// sale -las dos vistas y las dos exportaciones-, porque una barrera que cubre uno deja
// el otro abierto. Y el equipo tiene que seguir recibiendo la clave entera: el firmware
// la exige, y una app que tapa hacia el cable deja de mandar ordenes.
assert(sentFrames.length === 1 && sentFrames[0] === 'CMD:PIN:1234:SET_MODO:AMBAR\r\n',
  `Al cable sale el PIN ENTERO: ${JSON.stringify(sentFrames[0] || '')}`);
assert(R.todas().every(x => x.linea.indexOf('1234') < 0),
  'y en la cinta no queda un solo PIN en claro');
assert(D.todas().every(x => !x.linea || x.linea.indexOf('1234') < 0),
  'ni en el diario, que se exporta aparte');
document.getElementById('btn-depu-copiar').click();
document.getElementById('btn-diario-copiar').click();
assert(depuTexto.value.indexOf('CMD:PIN:1234') < 0 && depuTexto.value.indexOf('CMD:PIN:****') >= 0,
  'La exportacion de la cinta lleva **** donde iba la clave');
assert(diarioTexto.value.indexOf('CMD:PIN:1234') < 0 && diarioTexto.value.indexOf('CMD:PIN:****') >= 0,
  'La exportacion del diario tambien: son dos ficheros distintos y los dos viajan');
const vistaDiario = depuLista.textContent + diarioLista.textContent;
assert(vistaDiario.indexOf('CMD:PIN:1234') < 0 && vistaDiario.indexOf('CMD:PIN:****') >= 0,
  'y la pantalla ensena **** con el telefono en la mano de cualquiera');

// -------------------------------------------------------------------------
// 10.3 LA RESPUESTA SE PEGA A SU ORDEN, Y LO QUE NO CASA NO SE REPARTE A OJO
// -------------------------------------------------------------------------
const ackAmbar = 'ACK,CMD:SET_MODO:AMBAR,RESULT:OK';
window._btSubscribeCb(`$${ackAmbar}*${xorNmea(ackAmbar)}\n`);
assert(!!ordenAmbar.respuesta && ordenAmbar.respuesta.cmd === 'SET_MODO:AMBAR',
  `El $ACK se pega a la orden que lo provoco: ${ordenAmbar.respuesta ? ordenAmbar.respuesta.cmd : '(sin respuesta)'}`);
assert(!!ordenAmbar.respuesta && ordenAmbar.respuesta.atribucion === 'EXACTA',
  `y el diario publica COMO caso, no solo que caso: ${ordenAmbar.respuesta ? ordenAmbar.respuesta.atribucion : '-'}`);
assert(D.todas().filter(x => x.clase === 'RESPUESTA_SUELTA').length === 0,
  'y no queda ninguna respuesta suelta cuando la casacion era posible');

// EL CONTROL QUE LE FALTA A TODA CASACION (CLAUDE.md 8.sexies): un diario que pegara
// CUALQUIER respuesta a la ultima orden aprobaria las tres lineas de arriba igual de
// bien. Un $ACK de un comando que esta app no ha mandado tiene que quedarse SUELTO.
const ackAjeno = 'ACK,CMD:REINICIAR_RELOJ,RESULT:OK';
window._btSubscribeCb(`$${ackAjeno}*${xorNmea(ackAjeno)}\n`);
const sueltas1 = D.todas().filter(x => x.clase === 'RESPUESTA_SUELTA');
assert(sueltas1.length === 1 && sueltas1[0].respuesta.cmd === 'REINICIAR_RELOJ',
  `Un $ACK de una orden que esta app no dio queda SUELTO en vez de pegarse a la ultima (${sueltas1.length})`);
assert(sueltas1.length === 1 && /otro telefono|mando de reles/.test(sueltas1[0].respuesta.motivoSuelta),
  'y con el motivo escrito: puede venir de otro telefono o del mando de reles, que es un dato y no un fallo');

// La segunda forma de casar, que es la de las ordenes con argumentos: el equipo
// contesta CMD:SET_TIEMPOS a un SET_TIEMPOS:3,4,25. Se marca POR_CABECERA y la frase lo
// dice, porque no es lo mismo que devuelva el literal entero.
document.querySelector('.nav-item[data-tab="tab-tiempos"]').click();
inputVerde.value = '3'; inputRojo.value = '4'; inputDespeje.value = '25';
btnAplicarTiempos.click();
document.querySelector('.nav-item[data-tab="tab-depuracion"]').click();
const ackTiempos = 'ACK,CMD:SET_TIEMPOS,RESULT:OK';
window._btSubscribeCb(`$${ackTiempos}*${xorNmea(ackTiempos)}\n`);
const ordenTiempos = D.todas().filter(x => x.clase === 'ORDEN' && x.orden.indexOf('SET_TIEMPOS') === 0).pop();
assert(!!ordenTiempos && !!ordenTiempos.respuesta && ordenTiempos.respuesta.atribucion === 'POR_CABECERA',
  `SET_TIEMPOS:3,4,25 casa con CMD:SET_TIEMPOS por cabecera: ${ordenTiempos && ordenTiempos.respuesta ? ordenTiempos.respuesta.atribucion : '(sin respuesta)'}`);
assert(!!ordenTiempos && D.textoRespuesta(ordenTiempos, Date.now()).includes('sin los argumentos que se mandaron'),
  'y la frase avisa de que el equipo contesto sin los argumentos: no es el literal entero');

// -------------------------------------------------------------------------
// 10.4 MANUAL:CAMBIAR_TURNO VUELVE COMO CAMBIAR_TURNO: EL UNICO ALIAS
// -------------------------------------------------------------------------
// Medido en Maestro/src/bluetooth.cpp:524-536 -se recibe "MANUAL:CAMBIAR_TURNO" y se
// acusa "CAMBIAR_TURNO"-. Es la linea que mas facil se pierde en un refactor y la que
// mas cuesta perder: sin ella la unica orden de DAR PASO del operario quedaria SIEMPRE
// sin respuesta en el diario, y un hueco ahi se lee como averia del equipo.
sentFrames = [];
btnStep.click();
document.getElementById('btn-via-confirmar').click();
assert(sentFrames.some(f => f.indexOf('MANUAL:CAMBIAR_TURNO') >= 0),
  `Confirmada la via, DAR PASO sale al cable: ${sentFrames.join(' | ')}`);
const ackTurno = 'ACK,CMD:CAMBIAR_TURNO,RESULT:OK';
window._btSubscribeCb(`$${ackTurno}*${xorNmea(ackTurno)}\n`);
const ordenTurno = D.todas().filter(x => x.clase === 'ORDEN' && x.orden === 'MANUAL:CAMBIAR_TURNO').pop();
assert(!!ordenTurno && !!ordenTurno.respuesta && ordenTurno.respuesta.atribucion === 'POR_ALIAS',
  `y el $ACK,CMD:CAMBIAR_TURNO se le pega POR_ALIAS: ${ordenTurno && ordenTurno.respuesta ? ordenTurno.respuesta.atribucion : '(sin respuesta)'}`);
assert(!!ordenTurno && D.textoRespuesta(ordenTurno, Date.now()).includes('lo hace asi el firmware'),
  'y la frase dice que la traduccion es del firmware, no una interpretacion del diario');
// El control negativo del alias: que exista uno no puede volverse "casi todo casa".
assert(D.casa('CAMBIAR_TURNO', 'MANUAL:CAMBIAR_TURNO') === 'POR_ALIAS' &&
       D.casa('SET_MODO:AUTO', 'MANUAL:CAMBIAR_TURNO') === null &&
       Object.keys(D.ALIAS_CMD).length === 1,
  `Es el UNICO alias y no traduce nada mas: ${JSON.stringify(D.ALIAS_CMD)}`);

// -------------------------------------------------------------------------
// 10.5 AUTH_FAILED Y DESCONOCIDO NO NOMBRAN NINGUNA ORDEN
// -------------------------------------------------------------------------
// Medido con grep de '$ERR,CMD:' sobre los tres despachadores: el Maestro y el Esclavo
// emiten AUTH_FAILED y DESCONOCIDO sin el literal que se mando, y el puente ESP32 emite
// DESCONOCIDO. Con UNA sola orden esperando se atribuye Y SE DICE que fue por descarte;
// con dos o mas no se atribuye a ninguna. Un diario que reparte respuestas a ojo es peor
// que uno con huecos, porque el hueco se ve.
document.getElementById('btn-diario-limpiar').click();
darAmbar();
const soloUna = D.todas().filter(x => x.clase === 'ORDEN').pop();
const errAuth = 'ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO';
window._btSubscribeCb(`$${errAuth}*${xorNmea(errAuth)}\n`);
assert(!!soloUna && !!soloUna.respuesta && soloUna.respuesta.atribucion === 'POR_DESCARTE',
  `Con UNA sola orden esperando, el rechazo que no la nombra se le atribuye: ${soloUna && soloUna.respuesta ? soloUna.respuesta.atribucion : '(sin respuesta)'}`);
assert(!!soloUna && D.textoRespuesta(soloUna, Date.now()).includes('ATRIBUIDA POR DESCARTE'),
  'y el diario DICE que fue por descarte y que puede no ser de esa orden');

document.getElementById('btn-diario-limpiar').click();
darAmbar();
darAmbar();
const dosPendientes = D.todas().filter(x => x.clase === 'ORDEN');
assert(dosPendientes.length === 2 && dosPendientes.every(x => !x.respuesta),
  `Dos ordenes salen y las dos quedan esperando: ${dosPendientes.length}`);
window._btSubscribeCb(`$${errAuth}*${xorNmea(errAuth)}\n`);
assert(dosPendientes.every(x => !x.respuesta),
  'Con DOS esperando, AUTH_FAILED no se le cuelga a ninguna: seria adivinar');
const sueltas2 = D.todas().filter(x => x.clase === 'RESPUESTA_SUELTA');
assert(sueltas2.length === 1 && sueltas2[0].respuesta.motivoSuelta.indexOf('2 esperando') >= 0,
  `y queda suelta diciendo CUANTAS habia esperando: ${sueltas2.length ? sueltas2[0].respuesta.motivoSuelta.slice(-60) : '-'}`);
const errDesc = 'ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO';
window._btSubscribeCb(`$${errDesc}*${xorNmea(errDesc)}\n`);
assert(D.todas().filter(x => x.clase === 'RESPUESTA_SUELTA').length === 2 &&
       dosPendientes.every(x => !x.respuesta),
  'DESCONOCIDO se trata igual que AUTH_FAILED: los dos estan censados como "no nombran la orden"');

// -------------------------------------------------------------------------
// 10.6 EL EFECTO NO AFIRMA SOBRE DATOS QUE NO TIENE
// -------------------------------------------------------------------------
// La unica frase que AFIRMA es "NO CAMBIO NADA", y solo se llega ahi con un $STATUS
// antes, otro despues y los mismos valores en los dos. Las otras cuatro son "no lo se"
// dicho de cuatro maneras. Y todas publican cuantos segundos y cuantas tramas se
// miraron, porque la ventana esta atada a DESPEJE_SEG_MAX del C++ y si ese techo sube,
// el que lea el registro tiene que poder ver que la ventana no daba para la maniobra.
document.getElementById('btn-diario-limpiar').click();
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:MANUAL,ESTADO:R1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:15:01:00');
darAmbar();
const oEfecto = D.todas().filter(x => x.clase === 'ORDEN').pop();
const t0 = oEfecto.ms;
assert(D.textoEfecto(oEfecto, t0).indexOf('todavia no se puede ver') >= 0 &&
       D.textoEfecto(oEfecto, t0).indexOf('no ha llegado ningun $STATUS') >= 0,
  `Recien salida y sin trama detras, el efecto declara el silencio en vez de decir "sin cambio": ${D.textoEfecto(oEfecto, t0)}`);
for (let i = 0; i < 3; i++) {
  conectarComo('MAESTRO', `SERIE:SEM-M-01,MODO:MANUAL,ESTADO:R1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:15:0${2 + i}:00`);
}
const frEspera = D.textoEfecto(oEfecto, t0 + 1000);
assert(frEspera.indexOf('sin cambio todavia') >= 0 && frEspera.indexOf('3 $STATUS') >= 0 &&
       frEspera.indexOf('se mira hasta 95,0 s') >= 0,
  `Con tramas detras y la ventana abierta dice "todavia", y publica 3 tramas y los segundos: ${frEspera}`);
const frCerrada = D.textoEfecto(oEfecto, t0 + D.VENTANA_EFECTO_MS + 1);
assert(frCerrada.indexOf('NO CAMBIO NADA') >= 0 && frCerrada.indexOf('3 $STATUS de los 95,0 s') >= 0,
  `Cerrada la ventana con $STATUS a los dos lados SI afirma, y con la cuenta al lado: ${frCerrada}`);
// El control positivo: una frase que no supiera decir CAMBIO aprobaria las tres de
// arriba igual de bien -seria un diario que siempre dice que no paso nada-.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:MANUAL,ESTADO:V1_R2,T:31,RF:97,RTT:70,BAT:12.9,HORA:15:05:00');
assert(D.textoEfecto(oEfecto, t0 + D.VENTANA_EFECTO_MS + 1).indexOf('CAMBIO: ESTADO: R1_R2 -> V1_R2') >= 0,
  `y cuando algo SI se mueve lo nombra campo a campo: ${D.textoEfecto(oEfecto, t0 + D.VENTANA_EFECTO_MS + 1)}`);
// Y la mitad que mas duele: sin $STATUS con el que comparar, ni siquiera con la ventana
// cerrada se puede escribir "no cambio nada". Vaciar el diario borra el ultimo $STATUS
// visto, asi que esta orden sale sin "antes".
document.getElementById('btn-diario-limpiar').click();
darAmbar();
const oSinAntes = D.todas().filter(x => x.clase === 'ORDEN').pop();
const frSinDatos = D.textoEfecto(oSinAntes, oSinAntes.ms + D.VENTANA_EFECTO_MS + 1);
assert(frSinDatos.indexOf('NO SE PUDO VER') >= 0 && frSinDatos.indexOf('NO CAMBIO NADA') < 0,
  `Sin un $STATUS con el que comparar se DECLARA que no se pudo ver, no se afirma: ${frSinDatos}`);

// -------------------------------------------------------------------------
// 10.7 UNA ORDEN FRENADA ENTRA EN EL DIARIO Y NO EN LA CINTA
// -------------------------------------------------------------------------
// Para el que esta de pie delante del poste, "pulse y no paso nada" es el mismo sintoma
// tanto si la app se planto como si el equipo no contesto, y son averias distintas. Por
// eso la orden que no salio TAMBIEN se anota. Lo que no se hace es meterla en la cinta:
// por el cable no paso un byte, y la cinta es el cable.
//
// Se mide sobre una app recien montada porque en la de arriba el PIN ya esta tecleado y
// no queda ninguna barrera que frenar. SET_TIEMPOS es el llamador sin guarda propia mas
// facil de ejercer: lo para el `if` de dentro del emisor.
//
// LO QUE ESTA LINEA NO CUBRE, y va escrito para que no se lea de mas: de los dos motivos
// que ese `if` sabe redactar -"falta la autorizacion con PIN" y "falta confirmar que la
// via esta despejada"- solo el primero es alcanzable desde la interfaz. Censados los
// catorce llamadores de enviarComandoFirmware(), los cuatro que llegan a la guarda sin
// PIN propio -SET_MODO:DEGRADADO, SET_TIEMPOS y los dos SET_RTC- no estan en
// VIA_MANIOBRA, y los TRES que si lo estan -SET_MODO:AUTO, MANUAL:CAMBIAR_TURNO y,
// desde N-148, SET_MODO:AMBAR- sellan el vale y repiten el click en el mismo tick, asi
// que la guarda los ve ya autorizados. El motivo de via queda sin ejercer.
//
// Y con AMBAR hay que decir ademas POR DONDE se exige, porque no es por el mismo sitio:
// su guarda de via vive SOLO en el oyente del boton, no en enviarComandoFirmware() -ese
// `if` se satisface con el PIN de la sesion, que AMBAR conserva-. Censados los catorce
// llamadores y los cinco data-cmd del HTML, `btn-op-amber` es el UNICO camino por el que
// SET_MODO:AMBAR llega al emisor, asi que una puerta basta. El dia que aparezca un
// segundo llamador, la pregunta por el tramo no viajara con el: hay que moverla al
// emisor o repetirla alli.
const frenada = montarAppLimpia();
const wF = frenada.d.defaultView;
frenada.d.getElementById('num-tiempo-verde').value = '5';
frenada.d.getElementById('num-tiempo-rojo').value = '5';
frenada.d.getElementById('num-tiempo-despeje').value = '30';
frenada.tramas.length = 0;
wF.RegistroCrudo.limpiar();
wF.DiarioOrdenes.limpiar();
frenada.d.getElementById('btn-aplicar-tiempos').click();
assert(frenada.tramas.length === 0,
  `Sin PIN el emisor se planta y no escribe un byte: ${frenada.tramas.join(' | ')}`);
const noSalio = wF.DiarioOrdenes.todas().filter(x => x.clase === 'ORDEN');
assert(noSalio.length === 1 && noSalio[0].salio === false,
  `pero la orden SI entra en el diario, marcada como no salida (${noSalio.length})`);
assert(noSalio.length === 1 && /PIN|via/.test(noSalio[0].motivoNoSalio || ''),
  `con el motivo por el que se freno, no un "no paso nada": ${noSalio.length ? noSalio[0].motivoNoSalio : '-'}`);
assert(noSalio.length === 1 &&
       wF.DiarioOrdenes.textoOrden(noSalio[0]).indexOf('no esta en la cinta') >= 0,
  'y el propio diario avisa de que esa orden no esta en la cinta, para que nadie la busque alli');
assert(wF.RegistroCrudo.todas().filter(x => x.veredicto === wF.RegistroCrudo.ENVIADA).length === 0,
  'y en la cinta no hay ni una ENVIADA: por el cable no paso un byte');
const cF = wF.DiarioOrdenes.contadores(Date.now());
assert(cF.ordenes === 1 && cF.noSalieron === 1 && cF.sinRespuesta === 0,
  `Los contadores la cuentan como "no llego a salir" y no como una orden sin respuesta: ${JSON.stringify(cF)}`);

// =========================================================================
// 11. EL ESTADO DEL OTRO POSTE EN LA CONSOLA DEL MAESTRO (N-149, 05/09)
// =========================================================================
// DECISION DEL RESPONSABLE: "cuando me conecto al maestro no me aparecen los estados
// del semaforo del esclavo... yo necesito que maestro me traiga los datos del esclavo".
// Y sobre el atajo que la app ofrecia en su lugar: "hay un problema porque ahi esta
// haciendo la conexion por Bluetooth, pero tendrias que caminar 1000 metros hasta el
// otro lado".
//
// El dato viaja en un campo nuevo del $STATUS del MAESTRO -ESC:<ROJO|VERDE|AMBAR|?>-
// y lo que este bloque mide NO es que se pinte: es que las CUATRO respuestas posibles
// se distingan en pantalla. Pintar es la parte facil; la que se cobra en la calzada es
// no inventar un color cuando no hay dato (CLAUDE.md 3.quinquies).
//
//   ESC:VERDE    se pinta la lampara del POSTE 2
//   ESC:?        el Maestro dice que NO LO SABE -radio entre postes caida-
//   sin ESC:     firmware anterior: la app no puede saber nada del otro poste
//   ESC:<otro>   un literal que la tabla no conoce se ensena EN CRUDO
//
// Los dos del medio se pintan igual de vacios y se DICEN distinto a proposito: uno
// manda a actualizar el firmware y el otro a mirar la radio.
const s2Red = document.getElementById('s2-red');
const s2Amber = document.getElementById('s2-amber');
const s2Green = document.getElementById('s2-green');
const s2Text = document.getElementById('s2-text');
const s1TextDom = document.getElementById('s1-text');
const phaseDescDom = document.getElementById('phase-desc');
assert(!!s2Red && !!s2Amber && !!s2Green && !!s2Text,
  'La columna del POSTE 2 existe en el DOM: sus tres lamparas y su texto');

// 11.1 CON DATO: la lampara del otro poste se enciende y el texto lo nombra.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:VERDE,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:00,ESC:ROJO');
assert(s2Red.classList.contains('active') && !s2Green.classList.contains('active') &&
       !s2Amber.classList.contains('active'),
  `Con ESC:ROJO se enciende la roja del POSTE 2 y solo ella: rojo=${s2Red.className} verde=${s2Green.className}`);
assert(s2Text.textContent.includes('ROJO'),
  `y el texto del POSTE 2 lo dice: "${s2Text.textContent}"`);
// EL CONTROL POSITIVO, sin el cual una app que encendiera SIEMPRE la roja aprobaria lo
// de arriba igual de bien (CLAUDE.md 8.sexies).
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:02,ESC:VERDE');
assert(s2Green.classList.contains('active') && !s2Red.classList.contains('active'),
  `Con ESC:VERDE cambia de lampara: la roja se apaga y se enciende la verde: rojo=${s2Red.className} verde=${s2Green.className}`);
assert(s2Text.textContent.includes('VERDE'),
  `y el texto del POSTE 2 lo sigue: "${s2Text.textContent}"`);
// Y las DOS columnas dicen cosas distintas a la vez, que es lo que se pidio: el poste
// conectado en ROJO y el otro en VERDE. Si una pintara a la otra, esto cae.
assert(s1TextDom.textContent.includes('ROJO'),
  `y a la vez el POSTE 1 pinta lo SUYO -ESTADO:ROJO-, no lo del otro: "${s1TextDom.textContent}"`);

// 11.2 CON '?': no se inventa color. Y se dice POR QUE, que es la mitad util.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:04,ESC:?');
assert(!s2Red.classList.contains('active') && !s2Amber.classList.contains('active') &&
       !s2Green.classList.contains('active'),
  'Con ESC:? no queda ninguna lampara del POSTE 2 encendida: el ultimo color no se repinta como si fuera de ahora');
assert(/SIN DATO/i.test(s2Text.textContent) && /no lo sabe/i.test(s2Text.textContent),
  `y la pantalla DICE que el POSTE 1 no lo sabe, en vez de quedarse muda: "${s2Text.textContent}"`);
assert(/enlace entre/i.test(phaseDescDom.textContent),
  `y la linea del centro nombra la causa que hay que ir a mirar: "${phaseDescDom.textContent}"`);

// 11.3 SIN EL CAMPO: un equipo con firmware anterior no puede dejar dato pegado.
// Se inyecta DESPUES de una trama con ESC bueno a proposito: el defecto que esto
// vigila no es "no pinta nada", es "sigue pintando lo de la trama anterior".
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:06,ESC:VERDE');
assert(s2Green.classList.contains('active'), 'control: la verde del POSTE 2 queda encendida antes de quitar el campo');
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:08');
assert(!s2Green.classList.contains('active'),
  'Una trama SIN ESC: apaga la lampara que dejo la anterior: un dato que dejo de venir no sigue en pantalla');
assert(/SIN DATO/i.test(s2Text.textContent) && /no lo publica/i.test(s2Text.textContent),
  `y se declara la carencia REAL -este equipo no publica el campo-, que manda a un sitio distinto que el '?': "${s2Text.textContent}"`);

// 11.4 UN LITERAL DESCONOCIDO SE ENSENA EN CRUDO, no se traga. Es lo mismo que hace
// pintarBadgeModo() con un MODO nuevo: sin esto, el dia que el firmware estrene un
// valor la columna se queda en blanco y eso es indistinguible de una radio caida.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:10,ESC:AMARILLO');
assert(s2Text.textContent.includes('AMARILLO'),
  `Un valor de ESC que la tabla no conoce sale EN CRUDO en la pantalla: "${s2Text.textContent}"`);
assert(!s2Red.classList.contains('active') && !s2Amber.classList.contains('active') &&
       !s2Green.classList.contains('active'),
  'y no se adivina una lampara para el: no reconocerlo y encender algo serian cosas opuestas');
// Y el campo VACIO no se confunde con el campo AUSENTE, que es la unica pareja de esta
// tabla que se puede colapsar sin que se note: un `ESC:` sin nada dentro SI llego, asi
// que acusar al equipo de tener firmware viejo taparia el unico sintoma que deja un %s
// vacio en el snprintf del C++.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:11,ESC:');
assert(!/no lo publica/i.test(s2Text.textContent),
  `Un ESC: vacio NO se lee como "este equipo no publica el campo": el campo vino: "${s2Text.textContent}"`);

// 11.5 CONTRA EL ESCLAVO NO CAMBIA NADA: esa punta no emite ESC: -no tiene a quien
// preguntarle- y su ventana es de DIAGNOSTICO. La columna del POSTE 1 conserva la
// frase de siempre. Sin esta linea, un `state.esc` que sobreviviera al cambio de poste
// pintaria en el POSTE 1 el dato que dejo un Maestro hace dos minutos.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:16:00:12,ESC:VERDE');
conectarComo('ESCLAVO', 'SERIE:SEM-E-01,MODO:SUBORDINADO,ESTADO:ROJO,T:--,RF:--,RTT:--,BAT:--,HORA:16:00:14');
assert(s1TextDom.textContent.includes('no viaja en esta trama'),
  `Contra el ESCLAVO la columna del POSTE 1 sigue declarando que ese dato no viaja: "${s1TextDom.textContent}"`);
assert(!s1TextDom.textContent.includes('VERDE'),
  'y NO hereda el ESC: que publico el Maestro hace dos tramas: es dato de otro equipo');

// =========================================================================
// 12. LA HORA DEL EQUIPO SE PINTA (N-150, 05/09)
// =========================================================================
// El campo HORA: llega en cada $STATUS desde hace meses y `state.hora` tenia UN
// escritor y CERO lectores. Lo destapo un censo, no una queja de campo. Y empieza a
// costar esta noche: el puente ESP32 ya sella la hora del DS3231 en la trama, asi que
// HORA: deja de ser "--:--:--" y trae hora buena.
//
// ESTE BLOQUE MIDE EL CAMINO QUE CORRE EN EL TELEFONO -la rama de $STATUS de app.js-,
// que NO es el que miden los dos ficheros de unitarios: aquellos llaman a
// NMEAParser.parseStatus(), que la app no usa para pintar (ver el comentario de
// js/nmea_parser.js). Sin estas lineas, la hora podia estar verde en tres suites y no
// aparecer en ninguna pantalla, que es exactamente como llego hasta hoy.
const equipoHoraEl = document.getElementById('equipo-hora');
assert(!!equipoHoraEl, 'La consola tiene donde ensenar la hora que el equipo dice tener');

// 12.1 HORA BUENA: se pinta tal cual, sin reformatear y SIN TRUNCAR por el split(':').
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:18:25:03,ESC:ROJO');
assert(equipoHoraEl.textContent === '18:25:03',
  `La hora del equipo se pinta ENTERA -no "18", que es lo que da un split(':') sin limite-: "${equipoHoraEl.textContent}"`);

// 12.2 EL EQUIPO DICE QUE NO LA TIENE. Es lo que escribe su firmware cuando
// reloj_enHora() es falso, y no es un hueco de la app: es un dato, y de los caros -de
// esa hora cuelga la autorizacion del Modo Degradado-.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:--:--:--,ESC:ROJO');
assert(/SIN HORA/i.test(equipoHoraEl.textContent) && !/18:25/.test(equipoHoraEl.textContent),
  `Con HORA:--:--:-- se declara que el equipo NO tiene hora puesta y NO se queda la anterior: "${equipoHoraEl.textContent}"`);

// 12.3 EL TERCER TEXTO, QUE ES EL CONTROL DE LOS DOS DE ARRIBA. Una pantalla que
// escribiera SIEMPRE "SIN HORA" aprobaria 12.2 sola, y una que escribiera siempre lo que
// venga aprobaria 12.1 sola. Con un valor que no es ni hora ni guiones tienen que salir
// TRES textos distintos, y ese es el unico resultado que no puede dar una constante.
//
// Y se ensena EN CRUDO por lo mismo que un MODO desconocido en el badge: el dia que el
// campo cambie de forma, un hueco en blanco es indistinguible de un enlace caido.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:AYER,ESC:ROJO');
assert(/AYER/.test(equipoHoraEl.textContent) && !/SIN HORA/i.test(equipoHoraEl.textContent),
  `Un HORA: que no es ni hora ni guiones sale EN CRUDO y no se disfraza de las otras dos: "${equipoHoraEl.textContent}"`);

// 12.4 Y LO QUE SE VE ANTES DE QUE NADIE HABLE lo pone el HTML, no el painter, asi que
// se comprueba en el HTML: una hora de relleno en el fuente se leeria como la del equipo
// durante los dos segundos que tarda el primer $STATUS -y para siempre si nunca llega-.
// No se mide sobre una app montada porque montarAppLimpia() inyecta un $STATUS al
// arrancar: ahi ya hay dato, y esta linea dejaria de medir lo que dice medir.
const marcaHoraInicial = htmlContent.match(/id="equipo-hora"[^>]*>([^<]*)</);
assert(!!marcaHoraInicial && !/\d{1,2}:\d{2}/.test(marcaHoraInicial[1]),
  `El HTML no nace con una hora de relleno donde va la del equipo: "${marcaHoraInicial ? marcaHoraInicial[1].trim() : '(no se hallo el elemento)'}"`);

// =========================================================================
// 13. LA PLUMA SE DIBUJA, Y ROJO+ARRIBA NO ES UNA AVERIA (N-153, 05/09)
// =========================================================================
// MEDIDO ANTES DE ESCRIBIR NADA: "talanquera|pluma|barrera" daba 28 coincidencias en
// app.js y NINGUNA era un estado -todas explican lo que hace un boton-. El operario no
// podia saber si la barrera estaba arriba o abajo AHORA. Y no era que la app lo
// ignorase: el equipo no lo publicaba.
//
// ESTE BLOQUE MIDE EL CAMINO QUE CORRE EN EL TELEFONO, igual que el 12: la rama de
// $STATUS de app.js y pintarPluma(). El viaje de ida y vuelta del campo por el parser
// lo mide simulador_app_bluetooth.py con dominio cerrado; lo que solo se puede medir
// aqui es lo que queda ESCRITO en la pantalla, que es lo unico que lee el operario.
const plumaDom = document.getElementById('pluma-estado');
assert(!!plumaDom, 'La consola tiene donde ensenar el estado de la talanquera');

// 13.1 ABAJO: lo normal, y se dice sin adornos.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:17:00:00,ESC:VERDE,PLUMA:ABAJO');
assert(/PLUMA ABAJO/.test(plumaDom.textContent) && !/ARRIBA/.test(plumaDom.textContent)
       && plumaDom.hidden === false,
  `Con PLUMA:ABAJO la fila lo dice y no dice lo contrario: "${plumaDom.textContent}"`);

// 13.2 ARRIBA CON VERDE: es el ciclo normal, y NO se le pone encima el aviso de averia.
// Sin esta linea, una app que escribiera SIEMPRE el aviso aprobaria 13.3 sola.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:VERDE,T:20,RF:97,RTT:70,BAT:12.9,HORA:17:00:02,ESC:ROJO,PLUMA:ARRIBA');
assert(/PLUMA ARRIBA/.test(plumaDom.textContent) && !/AVER/i.test(plumaDom.textContent),
  `Con verde, la pluma arriba es el ciclo y no se anuncia como excepcion: "${plumaDom.textContent}"`);

// 13.3 ARRIBA CON ROJO: el caso que trae D-13 -una camara ve presencia debajo y la
// barrera no baja-. Hoy un operario eso lo lee como averia y llama. La pantalla lo tiene
// que DECIR, no insinuarlo, asi que se exige la palabra entera.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:17:00:04,ESC:VERDE,PLUMA:ARRIBA');
assert(/PLUMA ARRIBA/.test(plumaDom.textContent) && /NO es aver/i.test(plumaDom.textContent),
  `Con la luz en ROJO y la pluma ARRIBA la pantalla declara que no es una averia: "${plumaDom.textContent}"`);

// 13.4 EL CAMPO DEJA DE VENIR -firmware anterior a N-153-. No se puede quedar el ARRIBA
// de la trama de arriba pintado como si fuera de ahora: una barrera de hace un rato no
// es una barrera, y quien la lee cruza por debajo (CLAUDE.md 3.quinquies).
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:17:00:06,ESC:VERDE');
assert(/no la publica/i.test(plumaDom.textContent) && !/PLUMA ARRIBA/.test(plumaDom.textContent),
  `Una trama SIN PLUMA declara la carencia y borra el valor anterior: "${plumaDom.textContent}"`);

// 13.5 UN VALOR VACIO SI VINO, y no es lo mismo que no venir: acusar de firmware viejo a
// un equipo que acaba de mandar el campo taparia el unico sintoma de un %s vacio en el
// snprintf del C++. Es la misma distincion que ya se exige en 11.4 para ESC.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:17:00:08,ESC:VERDE,PLUMA:');
assert(!/no la publica/i.test(plumaDom.textContent),
  `Un PLUMA: vacio NO se lee como "este equipo no lo publica": el campo vino: "${plumaDom.textContent}"`);

// 13.6 UN LITERAL QUE LA APP NO CONOCE SE ENSENA EN CRUDO. Callarlo dejaria la fila en
// blanco el dia que el firmware estrene un valor, y una fila en blanco no se distingue
// de una app vieja.
conectarComo('MAESTRO', 'SERIE:SEM-M-01,MODO:AUTO,ESTADO:ROJO,T:20,RF:97,RTT:70,BAT:12.9,HORA:17:00:10,ESC:VERDE,PLUMA:ARIBA');
assert(/ARIBA/.test(plumaDom.textContent) && /NO RECONOCIDA/i.test(plumaDom.textContent),
  `Un valor de PLUMA que la tabla no conoce sale a la vista: "${plumaDom.textContent}"`);

// 13.7 LA PLUMA ES SIEMPRE LA DEL POSTE QUE HABLA, y el rotulo tiene que decirlo: las
// dos placas son la misma y las dos publican la SUYA. Un rotulo fijo mandaria al tecnico
// a mirar la barrera del otro poste, que esta a 1000 m.
conectarComo('ESCLAVO', 'SERIE:SEM-E-01,MODO:SUBORDINADO,ESTADO:ROJO,T:--,RF:--,RTT:--,BAT:--,HORA:17:00:12,PLUMA:ABAJO');
assert(/POSTE 2/.test(plumaDom.textContent) && /PLUMA ABAJO/.test(plumaDom.textContent),
  `Contra el ESCLAVO la fila habla del POSTE 2, que es donde esta esa barrera: "${plumaDom.textContent}"`);

console.log('='.repeat(80));
console.log(` RESULTADO JSDOM: ${testsPassed} PASS | ${testsFailed} FALLAS`);
console.log('='.repeat(80));

if (testsFailed > 0) {
  process.exit(1);
} else {
  process.exit(0);
}
