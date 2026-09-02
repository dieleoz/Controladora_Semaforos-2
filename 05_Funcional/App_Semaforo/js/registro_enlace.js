// ===== js/registro_enlace.js =====
//
// LA BITACORA DEL ENLACE. EL DATO QUE HOY NO EXISTE.
//
// EL PROBLEMA, DICHO POR EL RESPONSABLE: "el problema de desconexion es no saber
// CUANTO se va cuando se va, y POR QUE se va. Y eso no lo da la app."
//
// No lo daba, y el motivo es de calendario, no de pantalla: el campo RF: de $STATUS
// solo existe MIENTRAS ALGUIEN MIRA EL TELEFONO. En el momento de la caida el tecnico
// no esta delante -si estuviera, no haria falta la app para saber que se cayo-. Cuando
// llega, el enlace ya volvio, la pantalla dice 98% y no queda ni rastro de la hora a la
// que se fue ni de por donde venia bajando.
//
// Esto guarda esa tira: "a las 14:32 iba al 70, a las 14:35 al 40, a las 14:36 se fue,
// volvio a las 14:41". Persiste entre sesiones, se exporta para llevarsela, y guarda
// tambien los $EVENT y $ALARM del equipo CON EL VALOR DEL ENLACE AL LADO, que es la
// mitad que faltaba: una alarma sin saber como iba la radio en ese momento no dice si
// la causa fue la radio.
//
// LO QUE ESTE FICHERO TIENE PROHIBIDO HACER, Y ES SU RAZON DE SER
//
//   1. NO INTERPOLA. Si entre dos anotaciones no llego nada, el hueco se guarda como
//      hueco y se DIBUJA como hueco. Unir dos puntos con una linea recta es AFIRMAR
//      lo que paso en medio, y justo en medio es donde esta lo que se quiere saber:
//      un grafico que une el 70% de las 14:32 con el 40% de las 14:35 dibuja una
//      bajada suave que nadie midio, y puede haber sido una caida a cero y una
//      vuelta. Por eso `tramos()` mete un item HUECO en vez de estirar el anterior.
//
//   2. NO RELLENA UN VALOR QUE NO SE MIDIO CON UN CERO. Un 0 en la columna del enlace
//      significa "se midio y salieron cero latidos contestados", que es un dato
//      durisimo; "no se midio" no es eso, y no puede compartir casilla. Lo no medido
//      se guarda como `null` y sale como CELDA VACIA en el CSV. Es la misma regla que
//      el resto del repositorio: ABORTADO no es PASS.
//
//   3. NO CRECE SIN LIMITE. Esto vive en un telefono. Al pasar de TOPE se tira lo mas
//      VIEJO y se CUENTA cuanto se tiro, y la pantalla lo dice: un registro recortado
//      en silencio se lee como un registro completo, y entonces "no hay ninguna caida
//      antes de las 9" se confunde con "no guarde nada antes de las 9".
//
// LO QUE MIDE EL NUMERO QUE SE GUARDA, que no es lo que la gente supone al ver una
// barra de senal: `coordinador_calidadEnlace()` (Maestro/src/coordinador.cpp:845) es
// el PORCENTAJE DE LOS ULTIMOS 10 LATIDOS QUE CONTESTARON, uno cada 3 s, o sea una
// ventana de 30 s. NO ES UN RSSI. No hay dBm en ninguna parte del enlace y esta app no
// puede decir si la culpa es de la antena, del cable o de un camion aparcado delante:
// solo sabe cuantos latidos volvieron.

const RegistroEnlace = {

  // ---- Constantes. Se releen desde este fuente en el pack app_09. ----

  // Version en la clave: el dia que cambie la forma del registro, el viejo no se
  // interpreta mal -se queda aparte- en vez de leerse con el esquema nuevo.
  CLAVE: 'iotvial_registro_enlace_v1',

  // Tope duro de anotaciones. Con PERIODO_MUESTRA_MS eso es el horizonte minimo
  // garantizado del registro; los cambios de tramo y los eventos anaden mas
  // anotaciones, asi que en la practica cubre menos tiempo, nunca mas.
  TOPE: 400,

  // Cada cuanto se anota una MUESTRA de rutina. El equipo emite $STATUS cada segundo:
  // anotar uno por segundo llenaria el tope en 6 minutos y el registro no llegaria ni
  // al final del turno. Una muestra por minuto, MAS una cada vez que el enlace cambia
  // de tramo -que es el instante que interesa- y mas cada caida y cada vuelta.
  PERIODO_MUESTRA_MS: 60000,

  // A partir de aqui, dos anotaciones seguidas NO son continuas y entre ellas se
  // dibuja un hueco. Tiene que ser mayor que PERIODO_MUESTRA_MS o cada muestra de
  // rutina se dibujaria como una interrupcion; el pack recalcula esa desigualdad
  // desde estos dos numeros en vez de fiarse de este comentario, porque un comentario
  // no falla cuando alguien cambia uno de los dos (CLAUDE.md 3.bis, N-71).
  HUECO_MS: 150000,

  // RECHAZO (V2/A1): entro algo por el cable y la app NO lo pinto -checksum malo, tipo
  // desconocido, sin forma de trama-. Va en ESTA linea de tiempo y no en un registro
  // aparte porque es la misma pregunta: "por que se fue". Una tira que solo sabe de
  // huecos dice que a las 03:41 no llego nada; con esto se distingue "no llegaba nada"
  // de "llegaba basura", que son dos averias distintas -antena contra ruido- y desde el
  // suelo se ven igual.
  //
  // El CONTENIDO de cada trama rechazada no vive aqui: vive en la cinta de
  // js/depuracion.js, que es de memoria. Aqui va una anotacion ESTRANGULADA -ver
  // registrarRechazoEnlace() en app.js- porque mil tramas malas seguidas seguirian
  // siendo un solo suceso, y anotarlas una a una vaciaria el tope de 400 y se llevaria
  // por delante justo la historia del enlace que este fichero existe para guardar.
  CLASES: ['MUESTRA', 'CAIDA', 'REGRESO', 'EVENTO', 'ALARMA', 'RECHAZO'],
  CLASE_HUECO: 'HUECO',

  // Se pone a false la primera vez que el almacenamiento del telefono falla -modo
  // privado, cuota agotada, WebView sin permiso-. La pantalla lo DICE: un registro que
  // no se esta guardando y no lo avisa es peor que no tenerlo, porque el tecnico se va
  // creyendo que lleva la prueba encima.
  disponible: true,
  motivoNoDisponible: null,

  // ---- Almacen ----------------------------------------------------------

  _almacen() {
    // Se pregunta en cada llamada y no una vez al arrancar: en un WebView el
    // almacenamiento puede negarse mas tarde, y una bandera cacheada de hace una hora
    // no es una medida de ahora.
    try {
      if (typeof localStorage === 'undefined' || localStorage === null) return null;
      return localStorage;
    } catch (e) {
      return null;
    }
  },

  _vacio() {
    return { registros: [], descartados: 0 };
  },

  cargar() {
    const alm = this._almacen();
    if (!alm) {
      this.disponible = false;
      this.motivoNoDisponible = 'este telefono no deja guardar datos de la app';
      return this._vacio();
    }
    let crudo = null;
    try {
      crudo = alm.getItem(this.CLAVE);
    } catch (e) {
      this.disponible = false;
      this.motivoNoDisponible = 'no se pudo leer el almacenamiento: ' + e.message;
      return this._vacio();
    }
    if (!crudo) return this._vacio();
    let datos = null;
    try {
      datos = JSON.parse(crudo);
    } catch (e) {
      // Un registro ilegible NO se repara adivinando: se declara. Reconstruir a medias
      // produciria una tira con agujeros que parecen huecos de enlace y son de parser.
      this.disponible = false;
      this.motivoNoDisponible = 'el registro guardado esta corrupto y no se interpreta';
      return this._vacio();
    }
    if (!datos || !Array.isArray(datos.registros)) return this._vacio();
    return {
      registros: datos.registros.filter(r => r && typeof r.ms === 'number'),
      descartados: typeof datos.descartados === 'number' ? datos.descartados : 0
    };
  },

  _guardar(estado) {
    const alm = this._almacen();
    if (!alm) {
      this.disponible = false;
      this.motivoNoDisponible = 'este telefono no deja guardar datos de la app';
      return false;
    }
    try {
      alm.setItem(this.CLAVE, JSON.stringify(estado));
      this.disponible = true;
      this.motivoNoDisponible = null;
      return true;
    } catch (e) {
      this.disponible = false;
      this.motivoNoDisponible = 'no cabe en el telefono: ' + e.message;
      return false;
    }
  },

  // ---- Anotacion --------------------------------------------------------

  // clase   una de CLASES.
  // lectura {medido, pct, rtt} tal y como salio de la trama. Si `medido` es false, el
  //         enlace se guarda como null -NUNCA como 0-.
  // texto   lo que se leera en la lista. Es donde va el detalle del $EVENT/$ALARM.
  // ms      instante. Se admite por parametro para poder probarlo sin esperar.
  anotar(clase, lectura, texto, ms) {
    if (this.CLASES.indexOf(clase) < 0) return null;
    const lec = lectura || {};
    const reg = {
      ms: typeof ms === 'number' ? ms : Date.now(),
      clase: clase,
      // La condicion es `medido === true`, no la verdad del numero: un pct de 0 es un
      // dato legitimo -cero latidos contestados- y con `lec.pct ? ... : null` se
      // habria guardado como "no medido", que es la mentira contraria.
      rf: lec.medido === true && typeof lec.pct === 'number' ? lec.pct : null,
      rtt: lec.medido === true && typeof lec.rtt === 'number' ? lec.rtt : null,
      texto: String(texto === undefined || texto === null ? '' : texto)
    };
    const estado = this.cargar();
    estado.registros.push(reg);
    if (estado.registros.length > this.TOPE) {
      const sobran = estado.registros.length - this.TOPE;
      estado.registros.splice(0, sobran);
      estado.descartados += sobran;
    }
    this._guardar(estado);
    return reg;
  },

  limpiar() {
    const alm = this._almacen();
    if (!alm) return false;
    try {
      alm.removeItem(this.CLAVE);
      return true;
    } catch (e) {
      return false;
    }
  },

  // ---- Lectura para pintar ----------------------------------------------

  // Devuelve la secuencia que se dibuja: los registros en orden, CON UN ITEM HUECO
  // METIDO entre dos que estan separados mas de HUECO_MS.
  //
  // Esta funcion es el sitio donde este fichero se gana el sueldo. Lo facil es
  // devolver la lista tal cual y dejar que el dibujante ponga las barras pegadas: eso
  // pinta una tira continua sobre un tiempo en el que no se recibio nada, y quien la
  // mire leera "el enlace estuvo asi todo el rato". Un hueco no es un valor bajo ni un
  // valor alto: es la ausencia de medida, y se dibuja como su propia cosa.
  tramos(registros) {
    const lista = (registros || []).slice().sort((a, b) => a.ms - b.ms);
    const fuera = [];
    for (let i = 0; i < lista.length; i++) {
      if (i > 0) {
        const salto = lista[i].ms - lista[i - 1].ms;
        if (salto > this.HUECO_MS) {
          fuera.push({
            clase: this.CLASE_HUECO,
            ms: lista[i - 1].ms,
            hastaMs: lista[i].ms,
            duracionMs: salto,
            rf: null,
            rtt: null,
            texto: 'sin una sola trama durante ' + Math.round(salto / 1000) + ' s'
          });
        }
      }
      fuera.push(lista[i]);
    }
    return fuera;
  },

  // ---- Exportacion ------------------------------------------------------

  _celda(v) {
    // null -> celda VACIA. Un 0 aqui diria "se midio y dio cero", que es otra cosa.
    if (v === null || v === undefined) return '';
    return String(v);
  },

  _comillas(s) {
    return '"' + String(s).replace(/"/g, '""') + '"';
  },

  // El CSV lleva los HUECOS como filas propias, no como ausencia de filas: quien abra
  // el fichero en una hoja de calculo y dibuje una grafica con la columna del enlace
  // tiene que ver la interrupcion, o el programa de hojas de calculo hara por su
  // cuenta justo lo que este fichero se prohibe -unir los dos extremos con una recta-.
  aCsv(registros, descartados) {
    const filas = ['Fecha_ISO,Hora_local,Clase,Enlace_pct,RTT_ms,Detalle'];
    if (descartados) {
      filas.push(',,' + this._comillas('RECORTADO') + ',,,' +
                 this._comillas('faltan ' + descartados + ' anotaciones mas antiguas: ' +
                                'el tope del registro es ' + this.TOPE));
    }
    this.tramos(registros).forEach(r => {
      const d = new Date(r.ms);
      filas.push([
        d.toISOString(),
        this._comillas(d.toTimeString().split(' ')[0]),
        this._comillas(r.clase),
        this._celda(r.rf),
        this._celda(r.rtt),
        this._comillas(r.texto)
      ].join(','));
    });
    return filas.join('\n') + '\n';
  },

  // Horizonte MINIMO que cubre el registro, en horas, con las constantes de arriba.
  // Se calcula, no se escribe: si alguien cambia TOPE o el periodo, el texto de la
  // pantalla cambia con el en vez de quedarse describiendo una app que ya no existe.
  horizonteHoras() {
    return (this.TOPE * this.PERIODO_MUESTRA_MS) / 3600000;
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = RegistroEnlace;
}
