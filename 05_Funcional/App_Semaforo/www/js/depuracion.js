// ===== js/depuracion.js =====
//
// LA CINTA DE TRAMAS EN CRUDO. LO QUE LLEGO, TAL Y COMO LLEGO.
//
// EL PROBLEMA, DICHO POR EL RESPONSABLE: "hoy el problema es no saber cuanto se va
// cuando se va, y por que se va."
//
// js/registro_enlace.js ya contesta a "cuanto": guarda la tira del enlace y los
// HUECOS. Lo que no guardaba nadie es el CONTENIDO -el "por que"-: la app parseaba
// cada linea, se quedaba con los campos que entendia y TIRABA LA LINEA. Un fallo
// intermitente no se diagnostica con un semaforo dibujado; se diagnostica con la
// trama que no cuadro, y esa trama no existia en ninguna parte diez milisegundos
// despues de llegar.
//
// ESTE FICHERO NO ES UN SEGUNDO REGISTRO AL LADO DEL OTRO, y la diferencia esta
// medida, no elegida:
//
//   js/registro_enlace.js   la LINEA DE TIEMPO. Pocas anotaciones, una por minuto o
//                           por cambio de tramo, PERSISTIDA en el telefono. Sobrevive
//                           al cierre de la app porque su trabajo es contar la noche
//                           que nadie miro.
//   este fichero            la CINTA. Todo lo que entro por el cable, en orden, sin
//                           interpretar, EN MEMORIA. No persiste, y la pantalla lo
//                           dice: a un $STATUS por segundo, persistir la cinta entera
//                           llenaria el almacenamiento del telefono en una hora y se
//                           llevaria por delante la linea de tiempo, que es la que de
//                           verdad hace falta al dia siguiente.
//
// El puente entre los dos existe y esta en app.js: cuando una trama se RECHAZA, eso
// si se anota en la linea de tiempo -clase RECHAZO-, porque "a las 03:41 empezo a
// llegar basura" es exactamente el dato que se busca al dia siguiente. Lo que no se
// anota alli es cada una de las mil tramas malas: ver el estrangulador de app.js.
//
// LO QUE ESTE FICHERO TIENE PROHIBIDO HACER
//
//   1. NO INVENTA UNA TRAMA DE EJEMPLO. Nunca. Si no ha llegado nada, la vista lo
//      DECLARA -"todavia no ha entrado ninguna trama"- y se queda vacia. Esta app ya
//      pago una vez el panel de demo que pintaba fases inventadas sobre los mismos
//      widgets que la telemetria, y su gemelo peor: el ciclo local que se animaba solo,
//      sin que nadie lo pulsara, en cuanto no habia equipo delante (N-75). Lo que
//      sustituye a un dato que no se tiene no es una simulacion: es decirlo.
//
//      (Los dos se nombran aqui POR SU EFECTO y no por el identificador que tenian:
//      test_funcional_app.py caza el regreso del simulador buscando esos literales en
//      todo el JavaScript, y no sabe distinguir codigo de prosa. Un comentario que
//      explica el defecto retirado haria fallar al instrumento que lo vigila -es lo
//      mismo que le paso a app_09 en su primera corrida, cinco FALLA acertando dentro
//      de los comentarios-. Se cita N-75, que no es un identificador de codigo.)
//
//   2. NO NORMALIZA LA LINEA. Se guarda lo que llego. Los caracteres de control se
//      ESCAPAN al pintarlos y al exportarlos -\r, \n, \t y los no imprimibles como
//      \xNN- porque un CR suelto en mitad de una lista rompe la lista y el tecnico ve
//      una trama distinta de la que entro; pero lo que se escapa es la REPRESENTACION,
//      no el dato: la longitud que se publica es la de la linea original.
//
//   3. NO RECORTA EN SILENCIO. Ni la cinta -al pasar de TOPE se tira lo mas viejo y se
//      CUENTA- ni la linea -si pasa de LARGO_MAX se corta y se dice cuanto se corto-.
//      Un registro recortado sin avisar se lee como completo.
//
//   4. NO DECIDE. Aqui no se valida nada: el veredicto lo trae quien llama, que es el
//      unico que sabe lo que la app hizo DE VERDAD con esa linea. Si este fichero
//      juzgara por su cuenta, la cinta diria "RECHAZADA" mientras la app la pintaba, y
//      un registro que no coincide con lo que paso es peor que no tenerlo.

const RegistroCrudo = {

  // ---- Constantes. Se releen desde este fuente en el pack de la app. ----

  // Tope duro de la cinta. Con un $STATUS por segundo esto son ~5 minutos de trafico
  // continuo; los rechazos y las respuestas caben de sobra dentro. No se sube "por si
  // acaso": cada linea son ~150 B vivos en el telefono y la cinta se mira EN EL POSTE,
  // con el equipo delante, no al dia siguiente. Para el dia siguiente esta la linea de
  // tiempo de js/registro_enlace.js, que si persiste.
  TOPE: 300,

  // Tope de una sola linea. El peor caso del firmware es el $ALARM con el tramo del
  // enlace: 128 B de payload mas el cierre (Maestro/src/bluetooth.cpp, el buffer de
  // 160). Se deja holgura para que una trama LEGITIMA nunca salga cortada -y para que
  // una cortada signifique de verdad "algo mando una linea absurda"-.
  LARGO_MAX: 400,

  // La ventana de los contadores. "12 rechazadas" no dice nada sin un "de cuando":
  // doce en cinco minutos es un enlace roto y doce en toda la manana es ruido normal.
  VENTANA_MS: 300000,

  // Los dos unicos veredictos, y significan lo que la APP HIZO, no lo que la trama
  // parece: ACEPTADA = se pinto, RECHAZADA = no se pinto nada de ella.
  ACEPTADA: 'ACEPTADA',
  RECHAZADA: 'RECHAZADA',

  // Los motivos de rechazo. La lista vive aqui y no en app.js porque la vista, el
  // exportador y el pack tienen que nombrar los mismos: un motivo que solo existe en
  // el sitio que lo escribe no se puede contar ni buscar.
  //
  // No hay un motivo "campo que no parsea", y la ausencia es deliberada: un campo
  // ilegible NO tumba la trama. La app pinta el resto y DECLARA ese campo -es toda la
  // regla de RF_NO_MEDIDO-, asi que esa trama entra como ACEPTADA con un reparo al
  // lado. Meterla en el saco de las rechazadas diria que no se pinto nada, y se pinto.
  //
  // Y TAMPOCO hay un motivo "no traia ningun campo", que fue el cuarto que se escribio
  // y se retiro antes de conectarlo. Un $STATUS con el checksum bueno y sin un solo
  // campo legible SIGUE SIENDO LA PRUEBA DE QUE EL EQUIPO ESTA VIVO Y HABLANDO -el
  // checksum dice que llego entero-, y tirarla se llevaria por delante justo esa
  // prueba, que es el latido del que cuelga el watchdog de la pantalla. Entra como
  // aceptada con su reparo. Un motivo que ningun camino puede producir no es un motivo:
  // es una casilla de la tabla que nunca se rellena y que se lee como "esto no pasa".
  MOTIVOS: {
    SIN_FORMA: 'no tiene forma de trama: falta el $ del principio o el * del checksum',
    CHECKSUM: 'checksum malo: la cuenta no da lo que la trama dice traer',
    TIPO_DESCONOCIDO: 'tipo de trama que esta app no sabe leer'
  },

  // ---- La cinta -----------------------------------------------------------

  // EN MEMORIA A PROPOSITO. Ver la cabecera. La vista lo dice con todas las letras
  // para que nadie se vaya del poste creyendo que se lleva esto encima.
  PERSISTE: false,

  _cinta: [],
  descartados: 0,

  limpiar() {
    this._cinta = [];
    this.descartados = 0;
  },

  // ---- Escapado: se toca la REPRESENTACION, nunca el dato ------------------

  // Un CR suelto en mitad de la lista parte la lista y ensena una trama que no es la
  // que entro. Se escapa al PINTAR y al EXPORTAR; en `_cinta` la linea sigue entera.
  escapar(s) {
    let fuera = '';
    const t = String(s);
    for (let i = 0; i < t.length; i++) {
      const c = t.charCodeAt(i);
      if (c === 13) { fuera += '\\r'; continue; }
      if (c === 10) { fuera += '\\n'; continue; }
      if (c === 9) { fuera += '\\t'; continue; }
      if (c < 32 || c === 127) {
        fuera += '\\x' + c.toString(16).toUpperCase().padStart(2, '0');
        continue;
      }
      fuera += t[i];
    }
    return fuera;
  },

  // ---- Anotacion ----------------------------------------------------------

  // linea      la trama TAL Y COMO LLEGO. No se limpia ni se recorta el terminador.
  // veredicto  { aceptada, motivo, detalle, tipo, reparos } - lo trae quien llamo,
  //            que es el unico que sabe lo que la app hizo con ella (regla 4).
  // ms         instante. Por parametro para poder probarlo sin esperar.
  anotar(linea, veredicto, ms) {
    const v = veredicto || {};
    const cruda = String(linea === undefined || linea === null ? '' : linea);
    const largo = cruda.length;
    const reg = {
      ms: typeof ms === 'number' ? ms : Date.now(),
      // `linea` es lo que se ensena; `largoOriginal` es lo que llego. Si el corte se
      // guardara sin el largo, una trama recortada seria indistinguible de una corta.
      linea: largo > this.LARGO_MAX ? cruda.slice(0, this.LARGO_MAX) : cruda,
      largoOriginal: largo,
      cortada: largo > this.LARGO_MAX,
      veredicto: v.aceptada === true ? this.ACEPTADA : this.RECHAZADA,
      // El motivo solo tiene sentido en una rechazada. En una aceptada es null y no
      // cadena vacia: null es "no aplica" y '' se lee como "motivo en blanco".
      motivo: v.aceptada === true ? null : (v.motivo || 'SIN_FORMA'),
      detalle: v.detalle === undefined || v.detalle === null ? '' : String(v.detalle),
      tipo: v.tipo === undefined || v.tipo === null ? '' : String(v.tipo),
      // Reparos: lo que se pinto CON PEGAS. Una trama aceptada cuyo RF venia como '--'
      // entro entera y se pinto; el reparo dice que ese campo no traia medida.
      reparos: Array.isArray(v.reparos) ? v.reparos.slice() : []
    };
    this._cinta.push(reg);
    if (this._cinta.length > this.TOPE) {
      const sobran = this._cinta.length - this.TOPE;
      this._cinta.splice(0, sobran);
      this.descartados += sobran;
    }
    return reg;
  },

  // ---- Lectura ------------------------------------------------------------

  // Las tramas mas recientes primero, que es el orden en el que se mira una cinta con
  // el equipo delante. `limite` acota lo que se pinta, NO lo que se guarda.
  recientes(limite) {
    const n = typeof limite === 'number' && limite > 0 ? limite : this._cinta.length;
    return this._cinta.slice(-n).reverse();
  },

  todas() {
    return this._cinta.slice();
  },

  // Los contadores de la ULTIMA VENTANA. Se recalculan de la cinta en cada llamada en
  // vez de mantenerse incrementados aparte: un contador que se lleva por su cuenta y
  // una lista que se recorta por el tope acaban discrepando, y entonces la pantalla
  // ensena "12 rechazadas" encima de una lista donde solo hay 3 -y quien lo lea creera
  // la lista o creera el numero, pero no puede saber cual-.
  contadores(ahora) {
    const t = typeof ahora === 'number' ? ahora : Date.now();
    const desde = t - this.VENTANA_MS;
    const dentro = this._cinta.filter(r => r.ms >= desde);
    const cuenta = {
      ventanaMs: this.VENTANA_MS,
      total: dentro.length,
      aceptadas: 0,
      rechazadas: 0,
      conReparos: 0,
      porMotivo: {},
      porTipo: {},
      // Cuantas de las que hay en la cinta se quedaron FUERA de la ventana. Sin esto,
      // una cinta llena de tramas viejas y una ventana vacia se ven igual.
      fueraDeVentana: this._cinta.length - dentro.length,
      descartados: this.descartados
    };
    dentro.forEach(r => {
      if (r.veredicto === this.ACEPTADA) {
        cuenta.aceptadas++;
        if (r.reparos.length) cuenta.conReparos++;
      } else {
        cuenta.rechazadas++;
        cuenta.porMotivo[r.motivo] = (cuenta.porMotivo[r.motivo] || 0) + 1;
      }
      const tipo = r.tipo || '(sin tipo)';
      cuenta.porTipo[tipo] = (cuenta.porTipo[tipo] || 0) + 1;
    });
    return cuenta;
  },

  // ---- Exportacion --------------------------------------------------------
  //
  // TEXTO PLANO Y NO CSV, y es una decision con motivo: una trama en crudo lleva comas
  // por dentro -son su separador de campos- y meterla en una celda obliga a comillarla
  // y a doblar las que ya trae. Lo que el tecnico manda del poste tiene que poder
  // leerse tal cual en cualquier sitio, incluido el cuerpo de un mensaje. La linea de
  // tiempo, que si es tabular, sigue saliendo en CSV por su propio boton.
  //
  // Y no hace falta internet para nada de esto: se compone aqui y se guarda o se copia.

  aTexto(ahora, cabecera) {
    const t = typeof ahora === 'number' ? ahora : Date.now();
    const c = this.contadores(t);
    const l = [];
    l.push('IOT-VIAL - CINTA DE TRAMAS EN CRUDO');
    if (cabecera) Object.keys(cabecera).forEach(k => l.push(k + ': ' + cabecera[k]));
    l.push('Exportado: ' + new Date(t).toISOString());
    l.push('');
    l.push('CONTADORES DE LOS ULTIMOS ' + Math.round(c.ventanaMs / 60000) + ' MINUTOS');
    l.push('  tramas entradas : ' + c.total);
    l.push('  aceptadas       : ' + c.aceptadas + (c.conReparos ? ' (' + c.conReparos + ' con algun campo sin medida)' : ''));
    l.push('  rechazadas      : ' + c.rechazadas);
    Object.keys(c.porMotivo).sort().forEach(m => {
      l.push('    - ' + m + ': ' + c.porMotivo[m] + '  (' + (this.MOTIVOS[m] || 'motivo sin descripcion') + ')');
    });
    if (c.fueraDeVentana) {
      l.push('  (' + c.fueraDeVentana + ' tramas mas viejas que la ventana siguen en la cinta y NO se cuentan arriba)');
    }
    if (c.descartados) {
      l.push('  RECORTADO: se tiraron las ' + c.descartados + ' tramas mas antiguas al ' +
             'llegar al tope de ' + this.TOPE + '. Lo de antes de eso no esta aqui.');
    }
    l.push('');
    l.push('LA CINTA, DE LA MAS ANTIGUA A LA MAS NUEVA');
    l.push('Los caracteres de control salen escapados (\\r, \\n, \\xNN); la trama entro tal cual.');
    l.push('');
    if (!this._cinta.length) {
      l.push('  (vacia: no ha entrado ninguna trama en esta sesion de la app)');
    }
    this._cinta.forEach(r => {
      const hora = new Date(r.ms).toTimeString().split(' ')[0];
      let fila = hora + '  [' + r.veredicto + ']';
      if (r.motivo) fila += ' ' + r.motivo;
      l.push(fila);
      l.push('    ' + this.escapar(r.linea) +
             (r.cortada ? '   <-- CORTADA: llegaron ' + r.largoOriginal + ' caracteres' : ''));
      if (r.detalle) l.push('    motivo: ' + r.detalle);
      r.reparos.forEach(x => l.push('    reparo: ' + x));
    });
    return l.join('\n') + '\n';
  },

  // Cuantos minutos de trafico continuo cubre la cinta en el peor caso, con el
  // $STATUS por segundo del firmware. Se calcula, no se escribe: si alguien cambia el
  // TOPE, el texto de la pantalla cambia con el.
  horizonteMinutos() {
    return this.TOPE / 60;
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = RegistroCrudo;
}
