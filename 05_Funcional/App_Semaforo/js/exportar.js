// ===== js/exportar.js =====
//
// QUE FIRMWARE LLEVA EL EQUIPO, Y SACAR LA SESION ENTERA COMO UN SOLO FICHERO.
//
// EL PROBLEMA DE CAMPO (28/09). El tecnico pegaba la cinta y el diario como TEXTO en
// WhatsApp: llegaban troceados, saturaban el chat, y la cabecera no decia que firmware
// llevaba el equipo. Una cinta sin ese dato no se puede atribuir a un commit (CLAUDE.md
// 0.2), asi que el que la recibe no sabe contra que fuente leerla.
//
// DOS SELLOS, NO UNO, y no se colapsan. EVT:VERSION lo emite el PUENTE -el ESP32, con su
// propio binario- y la respuesta a CMD:VERSION la da el CONTROLADOR -el STM32-. Se cargan
// por caminos distintos (USB contra SWD) y pueden no coincidir; eso es un dato, no una
// incoherencia (vigilante.cpp lo deja escrito junto al $EVENT). Por eso se guardan por
// NODE y la linea los nombra a los dos.
//
// '--' ES "SIN SELLAR", NO "SIN DATO": el binario se compilo sin git ni FW_HASH.txt y no
// sabe de que commit salio. Se escribe en palabra para que nadie lo lea como un hueco de
// la app.

const Exportar = {
  SIN_PEDIR: 'sin pedir (pulse Consultar version)',

  // Anota el sello si la trama es una de las tres que lo traen. Devuelve si anoto.
  anotarVersion(mapa, tipo, campos) {
    if (!mapa || !campos || campos.FW === undefined) return false;
    const esVersion = tipo === '$EVENT' ? campos.EVT === 'VERSION'
      : (tipo === '$ACK' || tipo === '$ERR') && campos.CMD === 'VERSION';
    if (!esVersion) return false;
    mapa[campos.NODE || 'SIN_NODE'] = campos.FW;
    return true;
  },

  // 29/09: LA LINEA DE EVENTOS QUE DICE EL SELLO. Antes solo iba a la cabecera del
  // fichero exportado y en pantalla no se veia (el funcional no pudo leer su version).
  lineaVersion(campos) {
    const quien = campos.NODE === 'PUENTE' ? 'Puente (ESP32)' : 'Controlador ' + (campos.NODE || '?');
    return 'Version cargada - ' + quien + ': FW ' + (campos.FW || '--');
  },

  // Si ya contesto el CONTROLADOR. El sello del puente no dice nada del STM32.
  controladorContesto(mapa) {
    return Object.keys(mapa || {}).some(n => n !== 'PUENTE');
  },

  // La cabecera de la cinta al exportarla (salio de app.js el 28/09 con el sello:
  // app.js no puede crecer). horaDe es el _horaDe de app.js.
  textoCinta(state, horaDe) {
    const _horaDe = horaDe;
    return RegistroCrudo.aTexto(Date.now(), {
      App: 'IOT-VIAL V9.0 (Controladora Semaforos)',
      Cruce: state.site,
      Equipo: state.node === null ? 'sin identificar (ningun $STATUS con NODE)' : state.node,
      Serie: state.serie === null ? 'sin identificar' : state.serie,
      Firmware: Exportar.textoFirmware(state.firmware),
      Modo: state.modo === null ? 'sin telemetria' : state.modo,
      Estado: state.estadoLuces === null ? 'sin telemetria' : state.estadoLuces,
      Hora_RTC: state.hora === null ? 'sin telemetria' : state.hora,
      Pluma: state.pluma === null ? 'sin telemetria' : state.pluma,
      Camaras: state.cam === null ? 'sin telemetria' : state.cam,
      Enlace: state.rfQuality === null
        ? 'no medido en esta sesion'
        : state.rfQuality + '% a las ' + _horaDe(state.rfMedidaMs)
    });
  },

  textoFirmware(mapa) {
    const nodos = Object.keys(mapa || {});
    if (!nodos.length) return this.SIN_PEDIR;
    const t = nodos.map(n => n + ' ' + (mapa[n] === '--' ? '-- (sin sellar)' : mapa[n]))
      .join(' | ');
    return this.controladorContesto(mapa) ? t : t + ' | controlador ' + this.SIN_PEDIR;
  },

  // IOTVIAL_<serie>_<AAAAMMDD-HHMMSS>.txt, hora LOCAL del telefono: es la que el tecnico
  // tiene en la cabeza. La serie se limpia porque va a un nombre de fichero.
  nombreFichero(serie, fecha) {
    const d = fecha instanceof Date ? fecha : new Date();
    const p = n => String(n).padStart(2, '0');
    const s = String(serie || 'SIN_SERIE').replace(/[^A-Za-z0-9_-]+/g, '_');
    return 'IOTVIAL_' + s + '_' + d.getFullYear() + p(d.getMonth() + 1) + p(d.getDate()) +
           '-' + p(d.getHours()) + p(d.getMinutes()) + p(d.getSeconds()) + '.txt';
  },

  // COMO SALE EL FICHERO, en orden, y devuelve por donde salio para que el toast no
  // afirme lo que la pagina no sabe.
  //   'NATIVO'       dentro de la APK, con @capacitor/filesystem y @capacitor/share: se
  //                  escribe en la cache de la app y se abre la hoja de compartir de
  //                  Android con el fichero como ADJUNTO (compartirNativo, abajo).
  //   'COMPARTIDO'   la hoja de compartir del sistema, con el fichero como ADJUNTO (Web
  //                  Share nivel 2: Chrome de Android fuera de la APK).
  //   'DESCARGA'     <a download> de un Blob: navegador de escritorio o Chrome.
  //   'SIN_SALIDA'   dentro de la APK SIN los dos plugins (una APK anterior al 28/09).
  //                  MEDIDO sobre @capacitor/android 6.2.1: el WebView no registra ningun
  //                  DownloadListener, asi que un <a download> no hace nada. Pulsar y no
  //                  ver nada seria mentir: se dice.
  salida(entorno) {
    const e = entorno || {};
    if (e.nativo) return e.pluginsNativos ? 'NATIVO' : 'SIN_SALIDA';
    if (e.puedeCompartirFicheros) return 'COMPARTIDO';
    return 'DESCARGA';
  },

  // Los dos plugins, o null. Sin bundler: el puente nativo de Capacitor publica cada plugin
  // registrado en window.Capacitor.Plugins (JSExport.getPluginJS). Hace falta que esten
  // LOS DOS: escribir sin poder compartir deja un fichero que nadie ve.
  pluginsNativos(cap) {
    const p = cap && cap.Plugins;
    if (!p || !p.Filesystem || !p.Share) return null;
    if (typeof p.Filesystem.writeFile !== 'function' || typeof p.Share.share !== 'function') {
      return null;
    }
    return { Filesystem: p.Filesystem, Share: p.Share };
  },

  // Escribe el texto en la cache de la app (sin permisos: Directory.Cache = 'CACHE') y lo
  // comparte como fichero. El FileProvider que usa @capacitor/share es
  // <applicationId>.fileprovider, y file_paths.xml ya declara <cache-path path=".">.
  // Resuelve 'COMPARTIDO' o 'CANCELADO' (el tecnico cerro la hoja: no es un fallo); un
  // fallo de escritura o de la hoja se propaga para que la app lo pinte en rojo.
  // 'COMPARTIDO' dice que Android devolvio la hoja, NO que el mensaje llego.
  compartirNativo(plugins, nombre, texto) {
    return plugins.Filesystem.writeFile({
      path: nombre, data: texto, directory: 'CACHE', encoding: 'utf8'
    }).then(r => {
      if (!r || !r.uri) throw new Error('writeFile no devolvio uri');
      return plugins.Share.share({ title: nombre, files: [r.uri], dialogTitle: 'Enviar ' + nombre });
    }).then(() => 'COMPARTIDO', err => {
      if (/cancel/i.test(String(err && err.message || err))) return 'CANCELADO';
      throw err;
    });
  },

  // EXPORTAR TODO: UN fichero, como ADJUNTO. Pegado como texto en WhatsApp llegaba
  // troceado y saturaba el chat; un adjunto llega entero. Diario primero -es lo que se
  // lee- y la cinta detras; cada uno con su cabecera, que lleva el firmware.
  // app: { vacio(), texto(), nombre(), showToast, addEvent }, de app.js.
  conectarBoton(boton, app) {
    if (!boton) return;
    boton.addEventListener('click', () => {
      if (app.vacio()) {
        app.showToast('No hay nada que exportar: ni ordenes ni tramas en esta sesion');
        return;
      }
      const texto = app.texto();
      const nombre = app.nombre();
      const cap = window.Capacitor;
      let fichero = null;
      try {
        fichero = new File([texto], nombre, { type: 'text/plain' });
      } catch (e) {
        // Un WebView viejo sin el constructor File: se cae a la descarga.
      }
      const plugins = Exportar.pluginsNativos(cap);
      const via = Exportar.salida({
        nativo: !!(cap && typeof cap.isNativePlatform === 'function' && cap.isNativePlatform()),
        pluginsNativos: !!plugins,
        puedeCompartirFicheros: !!(fichero && navigator.canShare &&
                                   navigator.canShare({ files: [fichero] }))
      });
      const sinSalida = (motivo) => {
        app.showToast('No se pudo adjuntar el fichero: use Copiar');
        app.addEvent('red', 'EXPORTAR COMO ARCHIVO fallo: ' + motivo +
                            '. Use Copiar en el diario y en la cinta.');
      };
      if (via === 'SIN_SALIDA') {
        sinSalida('esta APK no trae la pieza nativa de compartir');
      } else if (via === 'NATIVO') {
        Exportar.compartirNativo(plugins, nombre, texto).then(r => {
          app.showToast(r === 'CANCELADO' ? 'Envio cancelado: ' + nombre + ' no salio'
                                          : nombre + ' pasado a la app elegida; confirme alli el envio');
        }).catch(err => sinSalida(String((err && err.message) || err)));
      } else if (via === 'COMPARTIDO') {
        navigator.share({ files: [fichero], title: nombre }).catch(() => {});
        app.showToast('Elija la app para enviar ' + nombre);
      } else {
        const link = document.createElement('a');
        link.href = URL.createObjectURL(new Blob([texto], { type: 'text/plain;charset=utf-8;' }));
        link.download = nombre;
        link.click();
        app.showToast('Fichero pedido: ' + nombre + '. Si no aparece, use Copiar');
      }
    });
  },

  // QUE FIRMWARE LLEVA, PEDIDO UNA VEZ AL CONECTAR Y SOLO SI HACE FALTA. VERSION es SIN_PIN
  // y no cambia nada (ver SIN_PIN en app.js), asi que es la unica orden que se puede mandar
  // sin que nadie pulse. N-66 sigue en pie: no se pide NADA que el equipo no conozca. Solo
  // sale si el CONTROLADOR no ha contestado ya -el EVT:VERSION del puente es el sello del
  // ESP32, no el del STM32- y si el equipo esta hablando: escribir a un cable mudo pintaria
  // un SIN ENLACE en rojo por una orden que nadie dio. esperaMs es TIMEOUT_ENLACE_MS de
  // app.js: el mismo borde, para que el equipo haya podido demostrar que habla (un $STATUS)
  // antes de escribirle nada.
  pedirVersionAlConectar(state, enviar, addEvent, esperaMs) {
    const miEnlace = (state.enlaceN = (state.enlaceN || 0) + 1);
    setTimeout(() => {
      if (!state.connected || state.enlaceN !== miEnlace || !state.telemetriaViva ||
          Exportar.controladorContesto(state.firmware)) return;
      // Una orden que nadie pulso NO renueva el PIN: ultimaOrdenMs es el reloj de
      // inactividad de la autorizacion, y esto no es actividad de nadie.
      const actividad = state.ultimaOrdenMs;
      const salio = enviar('VERSION');
      state.ultimaOrdenMs = actividad;
      if (salio) {
        addEvent('cyan', 'Version del firmware pedida sola al conectar (no cambia ' +
                         'nada). El acuse trae el commit que este equipo tiene CARGADO.');
      }
    }, esperaMs);
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Exportar;
}
