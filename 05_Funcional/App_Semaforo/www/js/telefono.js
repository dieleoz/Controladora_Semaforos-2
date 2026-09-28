// ===== js/telefono.js =====
// Lo que la app necesita para usarse en un TELEFONO y no solo en un navegador de PC
// (APP-1 de OPTIMIZACIONES.md, medido en un telefono real con viewport CSS 320x647).
// Solo toca como se ve y como se sale de una ventana: no manda ninguna orden, no lee
// ninguna trama y no cambia que hace un boton. Tres piezas:
//
//   1. ATRAS DE ANDROID. Con una ventana abierta (conexion, PIN, sitio, degradado,
//      via) cierra ESA ventana; sin ninguna, doble atras en 2 s para salir.
//   2. LAS VENTANAS ABREN ARRIBA. La hoja de conexion abria a mitad de scroll con la
//      cabecera cortada.
//   3. LA BARRA DE ABAJO NO TAPA NADA. El hueco inferior del contenido se mide con la
//      altura REAL de la barra, no con una cifra escrita a mano.

const Telefono = (() => {
  // --- 1. ATRAS DE ANDROID ---------------------------------------------------------
  //
  // El boton atras lo publica @capacitor/app como evento 'backButton' en
  // window.Capacitor.Plugins.App. OJO con lo que hace el plugin (AppPlugin.java de
  // @capacitor/app 6.0.3): en cuanto esta instalado se queda con la tecla, y si nadie
  // escucha solo hace goBack() del WebView y NADA mas; la app ya no se va. Por eso aqui
  // se escucha SIEMPRE. Sin ventana abierta (decision del responsable, 28/09): el primer
  // atras solo avisa; un segundo dentro de PLAZO_SALIR_MS sale con App.exitApp(), que
  // cierra la actividad (antes minimizeApp() la dejaba viva). Que pasa con el enlace
  // Bluetooth al salir no esta medido en telefono. Un atras que cierra ventana no cuenta.
  //
  // La ventana se cierra con un clic sobre su propio fondo, que es el camino que app.js
  // ya escucha para las cinco -`e.target === m`- y el que limpia lo que cada una deja
  // pendiente (el PIN tecleado, la orden en espera de via). No hay un segundo cierre
  // que mantener.
  function ventanaAbierta(doc) {
    const abiertas = doc.querySelectorAll('.modal-overlay.active');
    return abiertas.length ? abiertas[abiertas.length - 1] : null;
  }

  const PLAZO_SALIR_MS = 2000;
  const AVISO_SALIR = 'Pulse otra vez para salir';
  let avisoEn = null;          // cuando se dio el ultimo aviso; null = ninguno vigente

  // El toast es el de app.js (#toast-msg); su showToast() no se exporta, asi que se
  // escribe igual: texto, clase `show` y se retira pasado el plazo.
  function avisar(doc) {
    const t = doc.getElementById('toast-msg');
    if (!t) return;
    t.textContent = AVISO_SALIR;
    t.classList.add('show');
    doc.defaultView.setTimeout(() => {
      if (t.textContent === AVISO_SALIR) t.classList.remove('show');
    }, PLAZO_SALIR_MS);
  }

  // Devuelve lo que hizo, para que la prueba lo pueda aseverar: 'cerrada', 'aviso',
  // 'salida' o 'nada' (sin plugin al que pedirselo).
  function alPulsarAtras(doc, app) {
    const v = ventanaAbierta(doc);
    if (v) {
      avisoEn = null;
      v.dispatchEvent(new doc.defaultView.MouseEvent('click', { bubbles: true }));
      return 'cerrada';
    }
    if (!app || typeof app.exitApp !== 'function') return 'nada';
    const ahora = doc.defaultView.Date.now();
    if (avisoEn !== null && ahora - avisoEn < PLAZO_SALIR_MS) {
      avisoEn = null;
      app.exitApp();
      return 'salida';
    }
    avisoEn = ahora;
    avisar(doc);
    return 'aviso';
  }

  function instalarAtras(win) {
    const cap = win.Capacitor;
    const app = cap && cap.Plugins && cap.Plugins.App;
    if (!app || typeof app.addListener !== 'function') return false;
    app.addListener('backButton', () => alPulsarAtras(win.document, app));
    return true;
  }

  // --- 2. LAS VENTANAS ABREN ARRIBA ------------------------------------------------
  //
  // app.js abre una ventana poniendole la clase `active`. Se observa esa clase en vez
  // de envolver openModal(): asi vale para las cinco sin tocar app.js, y para la
  // siguiente que se anada con la misma clase.
  function alAbrirArriba(overlay) {
    overlay.scrollTop = 0;
    overlay.querySelectorAll('.modal-card, .bt-device-list').forEach(n => { n.scrollTop = 0; });
  }

  function instalarVentanas(doc) {
    const Obs = doc.defaultView.MutationObserver;
    const overlays = doc.querySelectorAll('.modal-overlay');
    if (!Obs) return overlays.length;
    const obs = new Obs(cambios => {
      cambios.forEach(c => {
        const el = c.target;
        const antes = String(c.oldValue || '').split(/\s+/).indexOf('active') >= 0;
        if (el.classList.contains('active') && !antes) alAbrirArriba(el);
      });
    });
    overlays.forEach(o => obs.observe(o, { attributes: true, attributeFilter: ['class'],
                                           attributeOldValue: true }));
    return overlays.length;
  }

  // --- 3. LA BARRA DE ABAJO NO TAPA NADA -------------------------------------------
  //
  // La barra flota sobre el borde de abajo y su altura depende del ancho, de si los
  // rotulos caben, del tamano de letra que el telefono imponga y de la zona segura del
  // sistema. El hueco que el contenido deja abajo sale de medirla pintada: desde su
  // borde de ARRIBA hasta el fondo de la pantalla, que lo incluye todo, mas un margen.
  const MARGEN_PX = 12;

  function huecoBarra(doc) {
    const win = doc.defaultView;
    const barra = doc.querySelector('.bottom-nav');
    if (!barra || !barra.offsetHeight) return null;   // sin pintar (jsdom): el del CSS
    const arriba = barra.getBoundingClientRect().top;
    return Math.ceil(win.innerHeight - arriba + MARGEN_PX);
  }

  function ajustarHueco(doc) {
    const px = huecoBarra(doc);
    if (px !== null) doc.documentElement.style.setProperty('--hueco-barra', px + 'px');
    return px;
  }

  function instalarBarra(win) {
    const doc = win.document;
    ajustarHueco(doc);
    win.addEventListener('resize', () => ajustarHueco(doc));
    const barra = doc.querySelector('.bottom-nav');
    if (barra && typeof win.ResizeObserver === 'function') {
      new win.ResizeObserver(() => ajustarHueco(doc)).observe(barra);
    }
  }

  function instalar(win) {
    instalarVentanas(win.document);
    instalarBarra(win);
    return instalarAtras(win);
  }

  return { alPulsarAtras, instalarAtras, instalarVentanas, huecoBarra, ajustarHueco, instalar };
})();

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Telefono;
}
