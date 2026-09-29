// ===== js/serial_pc.js =====
// LA APP WEB EN UN PC: window.bluetoothSerial SOBRE WEB SERIAL (Chrome / Edge de escritorio).
//
// app.js solo habla con el poste por window.bluetoothSerial (cordova-plugin-bluetooth-serial,
// en la APK). Fuera de la APK ese objeto no existe y la app decia "Sin radio en este
// entorno". En Windows, un modulo Bluetooth SPP emparejado aparece como un puerto COM
// virtual, y Chrome/Edge lo abren con navigator.serial. Este fichero instala un
// window.bluetoothSerial con los metodos y la semantica de callbacks que app.js usa
// (censados con grep en app.js: list, connect, subscribe, write, disconnect; y
// discoverUnpaired, que NO se implementa a proposito: sin el, app.js dice que solo
// lista los emparejados, que es la verdad en un PC).
//
// Solo se activa si NO hay bluetoothSerial, NO es la APK (Capacitor/Cordova, donde el
// plugin llega despues de cargar la pagina) y el navegador tiene navigator.serial.
//
// El transporte es el de js/bluetooth_driver.js (conectarWebSerial a 9600, lectura por
// lineas, enviar): se reutiliza, no se copia. El cierre si se hace aqui: desconectar() de
// aquel no espera a que el lector suelte el puerto, y close() sobre un puerto con el
// flujo bloqueado falla sin que nadie lo vea.
//
// SIN PROBAR CON UN EQUIPO: que el COM del Bluetooth del PC entregue las tramas del poste.

(function instalarSerialPC() {
  if (typeof window === 'undefined' || window.bluetoothSerial) return;
  if (window.cordova || (window.Capacitor && window.Capacitor.isNativePlatform &&
                         window.Capacitor.isNativePlatform())) return;
  if (typeof navigator === 'undefined' || !navigator.serial) return;
  if (typeof BluetoothDriver !== 'function') return;

  const ID = 'SERIAL_PC';
  const NOMBRE = 'Puerto serie del PC (Bluetooth emparejado)';
  let drv = null;
  let alCaer = null;      // el failure de connect(): la semantica del plugin es que
                          // tambien avisa si el enlace se cae despues de abrir
  let suscrito = null;
  let cola = [];          // lineas que llegan antes de subscribe()

  const esperar = (ms) => new Promise((r) => setTimeout(r, ms));

  function textoError(e) { return (e && e.message) ? e.message : String(e || 'error'); }

  function caida(err) {
    const f = alCaer;
    alCaer = null;
    if (drv) drv.conectado = false;
    if (f) f(err);
  }

  window.bluetoothSerial = {
    esPuertoSeriePC: true,

    list(success, failure) {
      try { success([{ name: NOMBRE, address: ID, id: ID }]); } catch (e) { if (failure) failure(textoError(e)); }
    },

    // requestPort() tiene que correr dentro del gesto del usuario: app.js llama a connect()
    // desde el clic en la fila, sin esperas por medio (salvo cerrar un enlace anterior).
    connect(address, success, failure) {
      if (drv && drv.conectado) { if (failure) failure('Ya hay un puerto abierto'); return; }
      drv = new BluetoothDriver();
      cola = [];
      suscrito = null;
      let abierto = false;
      drv.setDataHandler((linea) => {
        if (suscrito) suscrito(linea); else if (cola.length < 50) cola.push(linea);
      });
      drv.setErrorHandler((err) => { if (abierto) caida('Puerto serie perdido: ' + err); });
      drv.conectarWebSerial().then(
        () => { abierto = true; alCaer = failure || null; if (success) success(); },
        (e) => {
          drv = null;
          const t = textoError(e);
          if (failure) failure(/No port selected|NotFoundError/i.test(t)
            ? 'no se eligio ningun puerto COM' : t);
        }
      );
    },

    // El plugin entrega los datos hasta el delimitador incluido. bluetooth_driver.js ya
    // corta por '\n' y devuelve cada linea con su '\n', que es lo que app.js pide.
    subscribe(delimitador, success, failure) {
      suscrito = success;
      const pendientes = cola;
      cola = [];
      pendientes.forEach((l) => success(l));
    },

    unsubscribe(success) { suscrito = null; if (success) success(); },

    write(data, success, failure) {
      if (!drv || !drv.conectado) { if (failure) failure('Sin puerto serie abierto'); return; }
      drv.enviar(String(data)).then(() => { if (success) success(); },
                                    (e) => { if (failure) failure(textoError(e)); });
    },

    isConnected(success, failure) {
      if (drv && drv.conectado) { if (success) success(); } else if (failure) failure('No conectado');
    },

    disconnect(success, failure) {
      const d = drv;
      drv = null;
      alCaer = null;       // un cierre pedido no es una caida
      suscrito = null;
      if (!d || !d.serialPort) { if (success) success(); return; }
      d.conectado = false;
      (async () => {
        try { if (d.serialReader) await d.serialReader.cancel(); } catch (e) { /* ya cerrado */ }
        // El pipeTo interno suelta el puerto un momento despues de cancelar el lector.
        let ultimo = null;
        for (let i = 0; i < 20; i++) {
          try { await d.serialPort.close(); return true; } catch (e) { ultimo = e; await esperar(100); }
        }
        throw ultimo;
      })().then(() => { if (success) success(); },
                (e) => { if (failure) failure('No se pudo cerrar el puerto: ' + textoError(e)); });
    },
  };
})();
