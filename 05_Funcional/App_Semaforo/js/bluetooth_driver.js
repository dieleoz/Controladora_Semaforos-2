// ===== js/bluetooth_driver.js =====
// Capa de Abstracción de Transporte: Nativo SPP (Cordova) / Web Bluetooth BLE / Web Serial

class BluetoothDriver {
  constructor() {
    this.tipoConexion = 'NONE'; // 'SPP_NATIVE', 'BLE', 'SERIAL', 'MOCK'
    this.conectado = false;
    this.onDataCallback = null;
    this.onStatusChangeCallback = null;
    this.onErrorCallback = null;

    // Web Bluetooth Objects
    this.bleDevice = null;
    this.bleServer = null;
    this.bleTxChar = null;
    this.bleRxChar = null;

    // Web Serial Objects
    this.serialPort = null;
    this.serialReader = null;
  }

  setDataHandler(cb) { this.onDataCallback = cb; }
  setStatusChangeHandler(cb) { this.onStatusChangeCallback = cb; }
  setErrorHandler(cb) { this.onErrorCallback = cb; }

  _notificarEstado(conectado, tipo, info = '') {
    this.conectado = conectado;
    this.tipoConexion = tipo;
    if (this.onStatusChangeCallback) {
      this.onStatusChangeCallback({ conectado, tipo, info });
    }
  }

  _notificarDato(linea) {
    if (this.onDataCallback) {
      this.onDataCallback(linea);
    }
  }

  _notificarError(err) {
    if (this.onErrorCallback) {
      this.onErrorCallback(err);
    }
  }

  /**
   * Conecta usando Bluetooth Clásico Nativo (Cordova Plugin en APK)
   */
  conectarNativoSPP(macAddress) {
    if (typeof window !== 'undefined' && window.bluetoothSerial) {
      window.bluetoothSerial.connect(
        macAddress,
        () => {
          this._notificarEstado(true, 'SPP_NATIVE', `Conectado a ${macAddress}`);
          window.bluetoothSerial.subscribe(
            '\n',
            (data) => this._notificarDato(data),
            (err) => this._notificarError(`Falla en lectura serial: ${err}`)
          );
        },
        (err) => {
          this._notificarEstado(false, 'NONE', 'Falla de conexión');
          this._notificarError(`No se pudo conectar a ${macAddress}: ${err}`);
        }
      );
    } else {
      this._notificarError('bluetoothSerial no está disponible en este entorno');
    }
  }

  /**
   * Conecta usando Web Bluetooth (Navegador Chrome con BLE)
   */
  async conectarWebBLE() {
    if (!navigator.bluetooth) {
      throw new Error('Web Bluetooth no está soportado en este navegador');
    }

    try {
      this.bleDevice = await navigator.bluetooth.requestDevice({
        filters: [{ services: ['0000ffe0-0000-1000-8000-00805f9b34fb'] }],
        optionalServices: ['0000ffe0-0000-1000-8000-00805f9b34fb']
      });

      this.bleServer = await this.bleDevice.gatt.connect();
      const service = await this.bleServer.getPrimaryService('0000ffe0-0000-1000-8000-00805f9b34fb');
      const characteristic = await service.getCharacteristic('0000ffe1-0000-1000-8000-00805f9b34fb');

      this.bleTxChar = characteristic;
      this.bleRxChar = characteristic;

      await this.bleRxChar.startNotifications();
      this.bleRxChar.addEventListener('characteristicvaluechanged', (e) => {
        const decoder = new TextDecoder('utf-8');
        const text = decoder.decode(e.target.value);
        this._notificarDato(text);
      });

      this._notificarEstado(true, 'BLE', this.bleDevice.name || 'Dispositivo BLE');
    } catch (e) {
      this._notificarEstado(false, 'NONE', 'Falla BLE');
      this._notificarError(e.message);
      throw e;
    }
  }

  /**
   * Conecta mediante Web Serial API (Cable USB-OTG a 9600 bps)
   */
  async conectarWebSerial() {
    if (!navigator.serial) {
      throw new Error('Web Serial API no está soportada en este navegador');
    }

    try {
      this.serialPort = await navigator.serial.requestPort();
      await this.serialPort.open({ baudRate: 9600 });
      this._notificarEstado(true, 'SERIAL', 'Puerto Serial USB');

      const decoder = new TextDecoderStream();
      this.serialPort.readable.pipeTo(decoder.writable);
      const inputStream = decoder.readable;
      this.serialReader = inputStream.getReader();

      (async () => {
        let buffer = '';
        while (true) {
          const { value, done } = await this.serialReader.read();
          if (done) break;
          buffer += value;
          const lines = buffer.split('\n');
          buffer = lines.pop();
          for (const line of lines) {
            if (line.trim()) this._notificarDato(line.trim() + '\n');
          }
        }
      })().catch(err => this._notificarError(`Error en lectura USB: ${err}`));

    } catch (e) {
      this._notificarEstado(false, 'NONE', 'Falla Serial USB');
      this._notificarError(e.message);
      throw e;
    }
  }

  /**
   * Envía un string / comando al semáforo
   */
  async enviar(trama) {
    if (!this.conectado && this.tipoConexion !== 'MOCK') {
      throw new Error('No hay conexión activa con el semáforo');
    }

    if (this.tipoConexion === 'SPP_NATIVE' && window.bluetoothSerial) {
      return new Promise((resolve, reject) => {
        window.bluetoothSerial.write(trama, resolve, reject);
      });
    }

    if (this.tipoConexion === 'BLE' && this.bleTxChar) {
      const encoder = new TextEncoder();
      return this.bleTxChar.writeValue(encoder.encode(trama));
    }

    if (this.tipoConexion === 'SERIAL' && this.serialPort && this.serialPort.writable) {
      const encoder = new TextEncoder();
      const writer = this.serialPort.writable.getWriter();
      await writer.write(encoder.encode(trama));
      writer.releaseLock();
      return;
    }

    if (this.tipoConexion === 'MOCK') {
      console.log('[MOCK TX]:', trama);
      return;
    }
  }

  /**
   * Desconecta el dispositivo actual
   */
  desconectar() {
    if (this.tipoConexion === 'SPP_NATIVE' && window.bluetoothSerial) {
      window.bluetoothSerial.disconnect();
    } else if (this.tipoConexion === 'BLE' && this.bleDevice && this.bleDevice.gatt.connected) {
      this.bleDevice.gatt.disconnect();
    } else if (this.tipoConexion === 'SERIAL' && this.serialPort) {
      if (this.serialReader) this.serialReader.cancel();
      this.serialPort.close();
    }
    this._notificarEstado(false, 'NONE', 'Desconectado');
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = BluetoothDriver;
}
