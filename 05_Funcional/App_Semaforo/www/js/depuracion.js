// ===== js/depuracion.js =====
//
// DOS REGISTROS QUE VIVEN AQUI Y NO SON EL MISMO. Se leen en este orden:
//
//   RegistroCrudo    LA CINTA. Byte a byte, las dos direcciones, en orden, sin
//                    interpretar. Es donde se mira "que salio exactamente".
//   DiarioOrdenes    EL DIARIO. Una entrada por ORDEN, con su terna orden / respuesta
//                    / efecto. Es donde se mira "y que paso". Empieza en su propia
//                    cabecera, mas abajo, con el porque de que sea un segundo registro.
//
// -------------------------------------------------------------------------------
// LA CINTA DE TRAMAS EN CRUDO. LO QUE PASO POR EL CABLE, TAL Y COMO PASO.
//
// HASTA EL 04/09 SOLO GRABABA LO QUE ENTRA, y eso costo una sesion de banco: la
// inyeccion de hora del Courier devolvia "rechazo por el equipo, formato invalido",
// se exporto la cinta entera -300 tramas- y LA ORDEN QUE SE MANDO NO ESTABA. El
// formato se dedujo leyendo el firmware por las dos puntas en vez de leerlo del
// registro. Un registro que solo graba una mitad de una conversacion no diagnostica
// esa conversacion: es §4 -la regla del instrumento- en su forma mas cara, porque
// aqui el buscador estaba bien y lo que faltaba era el dato.
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
//   2. NO NORMALIZA LA LINEA -CON UNA SOLA EXCEPCION, EL PIN, Y ESTA ESCRITA EN
//      taparPin() con su motivo-. Se guarda lo que llego. Los caracteres de control se
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

  // Los tres veredictos, y significan lo que la APP HIZO, no lo que la trama parece:
  // ACEPTADA = se pinto, RECHAZADA = no se pinto nada de ella, ENVIADA = la app la
  // escribio a la salida.
  //
  // ENVIADA es del 04/09 y respeta ese mismo criterio, que es la razon de que sea un
  // veredicto y no un campo aparte. "Se escribio a la salida" es EXACTAMENTE lo que la
  // app sabe de una orden suya: no dice que el equipo la recibiera -la radio puede
  // estar caida-, ni que la aceptara -eso lo dice $ACK- ni que la ejecutara -eso lo
  // dice $STATUS-. Es la misma frase que enviarComandoFirmware() tiene escrita encima
  // de su `return true`, y aqui no se estira ni un milimetro.
  //
  // POR QUE HACIA FALTA, y el caso es del banco del 04/09: la inyeccion de hora del
  // Courier devolvia "rechazo por el equipo, formato invalido". Se exporto la cinta
  // -300 tramas- y LA ORDEN QUE SE MANDO NO ESTABA EN ELLA, porque la cinta solo
  // grababa lo que ENTRA. El formato del comando se dedujo leyendo las dos puntas en
  // vez de leerlo; veinte minutos para un dato que ya habia pasado por este fichero.
  ACEPTADA: 'ACEPTADA',
  RECHAZADA: 'RECHAZADA',
  ENVIADA: 'ENVIADA',

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

  // ---- El PIN no sale del telefono ----------------------------------------
  //
  // LO QUE SE EXPORTA SE MANDA POR WHATSAPP. Ese es el uso real y no una hipotesis:
  // el tecnico baja del poste, abre el fichero y lo pega en un grupo. Hasta el 04/09
  // cada orden autenticada viajaba dentro de la cinta como `CMD:PIN:1234:SET_MODO:AUTO`
  // y ahi iba la clave del equipo, en claro, a un chat.
  //
  // TRES DECISIONES, Y LAS TRES TIENEN MOTIVO:
  //
  //   1. SE TAPA AL ANOTAR, NO AL PINTAR. Lo que no se guarda no se puede filtrar por
  //      un camino que nadie reviso -una exportacion futura, un volcado de consola, un
  //      informe de error-. En `_cinta` no hay un solo PIN.
  //   2. SE CONSERVA LA LONGITUD, un '*' por caracter. Asi `largoOriginal` sigue siendo
  //      verdad y una trama tapada no se distingue en tamano de la que salio: si
  //      alguien mide bytes sobre la cinta, mide los que se escribieron al cable.
  //   3. NO TOCA EL ENVIO. Esta funcion no la llama enviarComandoFirmware() antes de
  //      escribir: la llama DESPUES, sobre una copia para el registro. El equipo sigue
  //      recibiendo la clave entera, porque el firmware la exige.
  //
  // Y es IDEMPOTENTE a proposito: `CMD:PIN:****:` vuelve a salir `CMD:PIN:****:`. Un
  // registro que se tapa dos veces por dos caminos distintos no se corrompe.
  taparPin(s) {
    return String(s === undefined || s === null ? '' : s)
      .replace(/(CMD:PIN:)([^:\r\n]*)(:)/g,
               (todo, cab, pin, fin) => cab + '*'.repeat(pin.length) + fin);
  },

  // ---- Anotacion ----------------------------------------------------------

  // linea      la trama TAL Y COMO LLEGO -o, si sale, tal y como se escribio-. No se
  //            limpia ni se recorta el terminador; lo unico que se altera es el PIN,
  //            y por el motivo escrito en taparPin().
  // veredicto  { aceptada, enviada, motivo, detalle, tipo, reparos } - lo trae quien
  //            llamo, que es el unico que sabe lo que la app hizo con ella (regla 4).
  // ms         instante. Por parametro para poder probarlo sin esperar.
  anotar(linea, veredicto, ms) {
    const v = veredicto || {};
    // El tapado va AQUI y no en quien llama: un solo sitio por el que pasan las dos
    // direcciones. Sobre una trama que entra no cambia nada -ninguna punta emite
    // `CMD:PIN:`, medido con grep sobre los tres despachadores- y aun asi se aplica,
    // porque una barrera que solo cubre el camino que hoy trae el secreto deja de
    // cubrirlo el dia que aparezca un eco.
    const cruda = this.taparPin(linea);
    const largo = cruda.length;
    const reg = {
      ms: typeof ms === 'number' ? ms : Date.now(),
      // `linea` es lo que se ensena; `largoOriginal` es lo que llego. Si el corte se
      // guardara sin el largo, una trama recortada seria indistinguible de una corta.
      linea: largo > this.LARGO_MAX ? cruda.slice(0, this.LARGO_MAX) : cruda,
      largoOriginal: largo,
      cortada: largo > this.LARGO_MAX,
      // ENVIADA gana a las otras dos si viene marcada. No es una preferencia: una
      // trama no puede salir y entrar a la vez, y quien llama pone una sola de las
      // dos banderas. El orden hace que una llamada mal formada -las dos puestas- se
      // registre como lo que de verdad hizo la app con una linea que ella compuso.
      veredicto: v.enviada === true ? this.ENVIADA
                 : (v.aceptada === true ? this.ACEPTADA : this.RECHAZADA),
      // El motivo solo tiene sentido en una rechazada. En una aceptada o en una
      // enviada es null y no cadena vacia: null es "no aplica" y '' se lee como
      // "motivo en blanco".
      motivo: (v.enviada === true || v.aceptada === true)
              ? null : (v.motivo || 'SIN_FORMA'),
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
      // `total` son las ANOTACIONES de la ventana, las dos direcciones juntas: es la
      // cuenta de lineas que hay en la cinta y por eso vale `dentro.length`. Lo que
      // hay que mirar para hablar del equipo es `entradas`, que es lo que ENTRO. Van
      // separadas porque desde el 04/09 la cinta graba tambien lo que sale, y sumar
      // las dos daria un "llegaron 40 tramas" donde llegaron 25 y salieron 15.
      total: dentro.length,
      entradas: 0,
      enviadas: 0,
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
      if (r.veredicto === this.ENVIADA) {
        cuenta.enviadas++;
      } else if (r.veredicto === this.ACEPTADA) {
        cuenta.entradas++;
        cuenta.aceptadas++;
        if (r.reparos.length) cuenta.conReparos++;
      } else {
        cuenta.entradas++;
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
    l.push('  tramas entradas : ' + c.entradas);
    l.push('  aceptadas       : ' + c.aceptadas + (c.conReparos ? ' (' + c.conReparos + ' con algun campo sin medida)' : ''));
    l.push('  rechazadas      : ' + c.rechazadas);
    Object.keys(c.porMotivo).sort().forEach(m => {
      l.push('    - ' + m + ': ' + c.porMotivo[m] + '  (' + (this.MOTIVOS[m] || 'motivo sin descripcion') + ')');
    });
    // Lo que SALIO va en su propia linea y no sumado a lo de arriba. Una orden no es
    // una trama del equipo, y meterlas en la misma cuenta convertiria "hablo poco" en
    // "hablo mucho" segun cuanto hubiera pulsado el tecnico.
    l.push('  ordenes enviadas: ' + c.enviadas +
           '  (lo que la app escribio al cable; la terna orden/respuesta/efecto va en ' +
           'el DIARIO DE ORDENES, que se exporta aparte)');
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
    l.push('"<--" es lo que ENTRO por el cable; "-->" es lo que la app ESCRIBIO al cable.');
    l.push('El PIN de las ordenes sale tapado con asteriscos. Al equipo se le manda entero: solo se tapa aqui.');
    l.push('');
    if (!this._cinta.length) {
      l.push('  (vacia: no ha entrado ni salido ninguna trama en esta sesion de la app)');
    }
    this._cinta.forEach(r => {
      const hora = new Date(r.ms).toTimeString().split(' ')[0];
      const flecha = r.veredicto === this.ENVIADA ? '-->' : '<--';
      let fila = hora + '  ' + flecha + '  [' + r.veredicto + ']';
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

// =============================================================================
// EL DIARIO DE ORDENES.  UNA ENTRADA POR ORDEN, CON SU TERNA.
// =============================================================================
//
// LA FRASE QUE LO PIDIO, del responsable el 04/09: "cada comando debe ser capturado en
// los logs, asi sabras que envia, que responde, que se activa, sin mas".
//
// Son TRES datos y no uno, y el tercero es el unico que contesta a la pregunta que
// lleva abierta desde el banco. Puestos en la misma entrada y en el mismo orden, un
// defecto se lee sin diagnosticarlo:
//
//   18:14:26  ORDEN      CMD:PIN:****:SET_MODO:AUTO
//   18:14:26  RESPUESTA  $ACK,CMD:SET_MODO:AUTO,RESULT:OK   (+0,3 s)
//             EFECTO     NO CAMBIO NADA: MODO siguio en AUTO y ESTADO en R1_R2
//
// Eso es N-42 -"el Modo Automatico no mueve las luces"- visible en tres lineas, sin
// que nadie tenga que cruzar dos ficheros ni acordarse de nada.
//
// POR QUE ES UN SEGUNDO REGISTRO Y NO UNAS COLUMNAS MAS EN LA CINTA
//
// Por una medida, no por gusto: la cinta se corta a 300 tramas y en UNA sola sesion de
// banco se tiraron 379. A un $STATUS cada 2000 ms (Maestro/src/bluetooth.cpp, "ahora -
// tUltimaTelemetria >= 2000") la cinta cubre unos diez minutos, y el ruido de la
// telemetria expulsa antes que nada lo poco que hubo: las ordenes. Un diario de 120
// entradas cabe en la memoria que sobra y aguanta una jornada entera de pulsaciones,
// porque nadie pulsa mil veces.
//
// LO QUE ESTE DIARIO NO PUEDE SABER  -- y va escrito aqui para que no se lea como
// permiso, que es como se leyo la Caja Negra de N-73 en cuatro manuales:
//
//   1. NO SABE SI EL EQUIPO RECIBIO LA ORDEN. Sabe que la app la escribio a la salida.
//      Entre la salida y el micro hay un socket Bluetooth, el ESP32 y un cable: los
//      tres pueden tragarse la trama sin que este lado se entere.
//   2. NO SABE SI EL EQUIPO OBEDECIO. Solo ve lo que el equipo CUENTA por $STATUS. Un
//      "NO CAMBIO NADA" dice que MODO y ESTADO no se movieron en las tramas que
//      llegaron; NO dice que el firmware desobedeciera. Hay ordenes que tardan a
//      proposito -SET_MODO:MENU contesta SALIENDO_TODO_ROJO y el cambio no llega hasta
//      que acabe el despeje, que puede ser de 90 s (DESPEJE_SEG_MAX,
//      Maestro/src/modo_automatico.cpp)- y por eso la ventana de efecto es mas larga
//      que ese techo y la frase publica SIEMPRE cuanto tiempo y cuantas tramas mira.
//   3. NO SABE QUE PASO EN LAS LUCES. $STATUS trae MODO y ESTADO, que es lo que el
//      firmware DICE que esta haciendo. Lo que hay en el poste lo dicen seis pines y
//      eso no viaja por Bluetooth: se mira con los ojos o con el arnes de dos puntas.
//   4. NO SABE DE ORDENES QUE NO SALIERON DE ESTA APP. Otro telefono, el mando de
//      reles o los botones de la tarjeta mueven el equipo sin pasar por aqui: sus
//      efectos apareceran como cambios "sin orden que los pidiera", y sus $ACK caeran
//      en el diario como RESPUESTA SUELTA -que es un dato, no un fallo-.
//   5. NO ADIVINA A QUE ORDEN CONTESTA UN RECHAZO QUE NO LA NOMBRA. $ERR,CMD:AUTH_FAILED
//      y $ERR,CMD:DESCONOCIDO no traen el literal que se mando (medido: grep de
//      '$ERR,CMD:' sobre los tres despachadores). Si hay UNA sola orden esperando, se
//      le atribuye Y SE DICE que fue por descarte; si hay dos o mas, no se atribuye a
//      ninguna. Un diario que reparte respuestas a ojo es peor que uno con huecos.
//
// Y LA REGLA QUE MANDA SOBRE TODO LO ANTERIOR (CLAUDE.md 3.quinquies): lo que sustituye
// a un dato que no se tiene no es una simulacion, es decirlo. Sin respuesta se escribe
// "sin respuesta". Sin $STATUS con el que comparar se escribe "no se pudo ver". El
// "no cambio nada" SOLO se escribe si hubo $STATUS antes Y despues y los campos son
// los mismos.

const DiarioOrdenes = {

  // ---- Constantes ---------------------------------------------------------

  // Ordenes que caben. 120 pulsaciones son mas de las que se dan en una jornada de
  // banco entera; el tope existe para que el diario no crezca sin fin, no para
  // recortar de verdad. Si alguna vez recorta, se cuenta y se dice.
  TOPE: 120,

  // Cuanto se espera un $ACK/$ERR antes de escribir "sin respuesta". El firmware
  // contesta dentro del mismo despachador que atiende la orden, asi que lo unico que
  // hay en medio es el bombeo del puente ESP32. NO ESTA CRONOMETRADO CONTRA HARDWARE
  // -es el mismo hueco que ESP32_ARRANQUE_MEDIDO = 0 de N-117-, asi que se pone
  // holgado a proposito: pasarse solo retrasa el rotulo, quedarse corto miente.
  VENTANA_RESPUESTA_MS: 5000,

  // Cuanto se mira el $STATUS despues de una orden antes de cerrar el veredicto de
  // efecto. NO ES UN NUMERO ELEGIDO: tiene que cubrir la maniobra mas lenta que el
  // firmware puede contestar con un ACK y ejecutar despues, que es el todo-rojo de
  // despeje -DESPEJE_SEG_MAX = 90 s, Maestro/src/modo_automatico.cpp-. Con una
  // ventana mas corta, un SET_MODO:MENU aceptado y en curso saldria aqui como "no
  // cambio nada", que es acusar al firmware de lo que hace bien.
  //
  // Si algun dia sube ese techo en el C++, esta ventana se queda CORTA y el diario
  // empieza a cerrar veredictos antes de tiempo. Por eso la frase publica siempre los
  // segundos que miro: un lector puede ver que la ventana no daba para la maniobra.
  VENTANA_EFECTO_MS: 95000,

  // Antiguedad maxima del $STATUS que se usa como "lo que habia antes". El firmware
  // emite uno cada 2000 ms (Maestro/src/bluetooth.cpp), asi que tres periodos son de
  // sobra. Un $STATUS de hace medio minuto NO sirve de "antes": entre el y la orden
  // pudo cambiar todo, y comparar contra el produciria un cambio -o una igualdad- que
  // no tiene nada que ver con la orden.
  ANTES_MAX_MS: 6000,

  // Los campos de $STATUS con los que se mide el efecto. Son los dos que dicen QUE
  // ESTA HACIENDO el equipo. No se meten aqui T, RF, RTT ni BAT: se mueven solos cada
  // trama y cualquier orden "cambiaria" algo, que es la forma facil de tener un diario
  // que siempre dice que si.
  CAMPOS_EFECTO: ['MODO', 'ESTADO'],

  // LA UNICA ORDEN CUYO LITERAL NO VUELVE IGUAL, y esto es una AFIRMACION SOBRE EL
  // CODIGO, o sea algo que se mide (CLAUDE.md 3.bis, la lista de excepciones con
  // motivos sin verificar). Medido asi, y se vuelve a medir al heredarlo:
  //
  //   grep -n "CAMBIAR_TURNO" 01_Firmware/Maestro/src/bluetooth.cpp
  //     :524  } else if (strcmp(accion, "MANUAL:CAMBIAR_TURNO") == 0) {
  //     :533      enviarTramaConCrc("$ACK,CMD:CAMBIAR_TURNO,RESULT:OK");
  //     :536      enviarTramaConCrc("$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE");
  //
  // Se recibe "MANUAL:CAMBIAR_TURNO" y se contesta "CAMBIAR_TURNO". Sin esta entrada,
  // la unica orden de dar paso del operario quedaria SIEMPRE sin respuesta en el
  // diario, y eso es justo la clase de hueco que se lee como averia.
  //
  // Las demas casan solas por una de las dos reglas de casa(): identicas
  // (SET_MODO:AUTO, FORZAR_ROJO, AMBAR_EMERGENCIA, CANCELAR_AMBAR, SOLICITAR_PASO,
  // TEST_LEDS, DEMANDA, REINICIAR_RELOJ) o por cabecera, cuando la orden lleva
  // argumentos y el ACK no (SET_TIEMPOS:12,10,5 -> CMD:SET_TIEMPOS; SET_RTC:fecha,hora
  // -> CMD:SET_RTC).
  ALIAS_CMD: {
    'MANUAL:CAMBIAR_TURNO': 'CAMBIAR_TURNO'
  },

  // Los dos CMD que NO nombran ninguna orden. Medidos con el mismo grep sobre las tres
  // puntas: Maestro y Esclavo emiten "$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO" y
  // "$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO[_EN_ESCLAVO]"; el puente ESP32
  // emite "$ERR,CMD:DESCONOCIDO" en puente.cpp. Un rechazo de PIN no dice a que orden
  // se lo nego, asi que este diario tampoco lo dira salvo que quede una sola candidata.
  CMD_SIN_ORDEN: ['AUTH_FAILED', 'DESCONOCIDO'],

  // ---- Estado -------------------------------------------------------------

  // EN MEMORIA, como la cinta y por el mismo motivo. La vista lo declara.
  PERSISTE: false,

  _diario: [],
  descartados: 0,
  // El ultimo $STATUS visto, para poder decir QUE HABIA JUSTO ANTES de una orden. Se
  // guarda solo lo que se compara, no la trama: la trama entera ya esta en la cinta.
  _ultimoStatus: null,

  limpiar() {
    this._diario = [];
    this.descartados = 0;
    this._ultimoStatus = null;
  },

  _recortar() {
    if (this._diario.length > this.TOPE) {
      const sobran = this._diario.length - this.TOPE;
      this._diario.splice(0, sobran);
      this.descartados += sobran;
    }
  },

  _campos(data) {
    const c = {};
    const d = data || {};
    this.CAMPOS_EFECTO.forEach(k => {
      if (d[k] !== undefined && d[k] !== null) c[k] = String(d[k]);
    });
    return c;
  },

  // Segundos con un decimal y coma, que es como se leen aqui. "+0,3 s".
  _seg(ms) {
    return (Math.max(0, ms) / 1000).toFixed(1).replace('.', ',');
  },

  _hora(ms) {
    return new Date(ms).toTimeString().split(' ')[0];
  },

  // ---- Correlacion --------------------------------------------------------
  //
  // DEVUELVE COMO CASO, NO SOLO SI CASO. La forma de la casacion se publica en el
  // diario: no es lo mismo que el equipo devuelva el literal entero que que lo devuelva
  // recortado o cambiado. Quien lea el registro tiene derecho a saber cuanta
  // interpretacion hubo entre la orden y su respuesta.
  casa(cmd, orden) {
    if (!cmd || !orden) return null;
    if (cmd === orden) return 'EXACTA';
    if (cmd === String(orden).split(':')[0]) return 'POR_CABECERA';
    if (this.ALIAS_CMD[orden] === cmd) return 'POR_ALIAS';
    return null;
  },

  // ---- Anotacion ----------------------------------------------------------

  // orden   el literal de la orden tal y como viaja: "SET_MODO:AUTO", "SET_RTC:f,h".
  // linea   la trama entera que se escribio al cable, o null si NO se escribio.
  // ms      instante. Por parametro para poder probarlo sin esperar.
  // extra   { salio: false, motivo: '...' } cuando la app se planto y no envio.
  //
  // UNA ORDEN QUE NO SALIO TAMBIEN SE ANOTA, y es deliberado: para el que esta delante
  // del poste, "pulse y no paso nada" es el mismo sintoma tanto si la app se planto
  // como si el equipo no contesto, y son averias distintas. Lo que NO se hace es
  // meterla en la cinta: por el cable no paso nada, y la cinta es el cable.
  anotarOrden(orden, linea, ms, extra) {
    const e = extra || {};
    const t = typeof ms === 'number' ? ms : Date.now();
    const ult = this._ultimoStatus;
    const edad = ult ? t - ult.ms : null;
    const hayCampos = !!ult && Object.keys(ult.campos).length > 0;
    const usable = !!ult && edad >= 0 && edad <= this.ANTES_MAX_MS && hayCampos;
    const reg = {
      clase: 'ORDEN',
      ms: t,
      orden: String(orden === undefined || orden === null ? '' : orden),
      // El PIN se tapa AQUI tambien, y no solo en la cinta: los dos registros se
      // exportan por separado y una barrera que cubre uno deja el otro abierto.
      linea: (linea === undefined || linea === null)
             ? null : RegistroCrudo.taparPin(linea),
      salio: e.salio !== false,
      motivoNoSalio: e.salio === false
                     ? String(e.motivo || 'la app no dijo por que') : null,
      respuesta: null,
      // Lo que habia ANTES. null con su motivo escrito: sin esto, el veredicto de
      // efecto no se puede dar y hay que decirlo, no rellenarlo.
      antes: usable ? Object.assign({}, ult.campos) : null,
      antesMs: ult ? ult.ms : null,
      sinAntes: usable ? null : (
        !ult ? 'cuando salio la orden no habia llegado ningun $STATUS en esta sesion'
             : (!hayCampos
                ? 'el ultimo $STATUS anterior no traia ni MODO ni ESTADO'
                : 'el ultimo $STATUS anterior es de ' + this._seg(edad) + ' s antes de ' +
                  'la orden, y solo se admiten ' + this._seg(this.ANTES_MAX_MS) + ' s')),
      despues: null,
      despuesMs: null,
      statusEnVentana: 0,
      cambios: {}
    };
    this._diario.push(reg);
    this._recortar();
    return reg;
  },

  // Cada $STATUS hace dos cosas: alimenta la ventana de efecto de las ordenes que
  // siguen abiertas, y pasa a ser "lo que habia antes" de la siguiente orden.
  verStatus(data, ms) {
    const t = typeof ms === 'number' ? ms : Date.now();
    const campos = this._campos(data);
    for (let i = this._diario.length - 1; i >= 0; i--) {
      const r = this._diario[i];
      if (r.clase !== 'ORDEN' || !r.salio) continue;
      if (t < r.ms) continue;
      // El diario esta en orden de tiempo: en cuanto una orden queda fuera de la
      // ventana, las de mas atras tambien. Se corta en vez de recorrerlo entero.
      if (t - r.ms > this.VENTANA_EFECTO_MS) break;
      r.statusEnVentana++;
      r.despues = campos;
      r.despuesMs = t;
      if (!r.antes) continue;
      Object.keys(campos).forEach(k => {
        if (r.antes[k] === undefined) return;
        if (campos[k] !== r.antes[k] && r.cambios[k] === undefined) {
          r.cambios[k] = { de: r.antes[k], a: campos[k], ms: t };
        }
      });
    }
    this._ultimoStatus = { ms: t, campos: campos };
  },

  // tipo   '$ACK' o '$ERR'.
  // data   los campos ya partidos de la trama (CMD, RESULT, DESC, NODE...).
  // linea  la trama entera, para guardarla literal.
  //
  // Devuelve la entrada a la que fue a parar -la orden, o la entrada suelta que se crea
  // cuando no se puede atribuir-.
  verRespuesta(tipo, data, linea, ms) {
    const t = typeof ms === 'number' ? ms : Date.now();
    const d = data || {};
    const cmd = (d.CMD === undefined || d.CMD === null) ? '' : String(d.CMD);
    const paquete = {
      ms: t,
      tipo: String(tipo || ''),
      cmd: cmd,
      resultado: d.RESULT !== undefined ? String(d.RESULT) : '',
      motivo: d.DESC !== undefined ? String(d.DESC) : '',
      nodo: d.NODE !== undefined ? String(d.NODE) : '',
      linea: RegistroCrudo.taparPin(linea),
      atribucion: null,
      motivoSuelta: null
    };

    if (this.CMD_SIN_ORDEN.indexOf(cmd) < 0) {
      // El equipo nombra la orden: se busca la mas reciente que case y siga sin
      // respuesta. La mas reciente y no la mas vieja porque dos pulsaciones seguidas
      // de la misma orden se contestan en el mismo orden en que salieron, y quien
      // mira el registro busca la ultima.
      for (let i = this._diario.length - 1; i >= 0; i--) {
        const r = this._diario[i];
        if (r.clase !== 'ORDEN' || !r.salio || r.respuesta) continue;
        if (t < r.ms) continue;
        const como = this.casa(cmd, r.orden);
        if (como) {
          paquete.atribucion = como;
          r.respuesta = paquete;
          return r;
        }
      }
      paquete.motivoSuelta = 'ninguna orden de esta app esperaba respuesta al comando ' +
                             (cmd || '(vacio)') + '. Puede venir de otro telefono, del ' +
                             'mando de reles o de una orden anterior al arranque de la app';
    } else {
      // El equipo NO nombra la orden. Solo se atribuye si no hay ambiguedad.
      const pend = [];
      for (let i = this._diario.length - 1; i >= 0; i--) {
        const r = this._diario[i];
        if (r.clase !== 'ORDEN' || !r.salio || r.respuesta) continue;
        if (t < r.ms || t - r.ms > this.VENTANA_RESPUESTA_MS) continue;
        pend.push(r);
      }
      if (pend.length === 1) {
        paquete.atribucion = 'POR_DESCARTE';
        pend[0].respuesta = paquete;
        return pend[0];
      }
      paquete.motivoSuelta = pend.length === 0
        ? 'el rechazo ' + cmd + ' no nombra la orden a la que contesta, y no habia ' +
          'ninguna esperando respuesta: no se atribuye a nada'
        : 'el rechazo ' + cmd + ' no nombra la orden a la que contesta, y habia ' +
          pend.length + ' esperando: no se atribuye a ninguna, porque seria adivinar';
    }

    const suelta = { clase: 'RESPUESTA_SUELTA', ms: t, respuesta: paquete };
    this._diario.push(suelta);
    this._recortar();
    return suelta;
  },

  // ---- Veredictos ---------------------------------------------------------

  estadoRespuesta(reg, ahora) {
    if (!reg || reg.clase !== 'ORDEN') return 'NO_APLICA';
    if (!reg.salio) return 'NO_SALIO';
    if (reg.respuesta) return 'LLEGO';
    const t = typeof ahora === 'number' ? ahora : Date.now();
    return (t - reg.ms) <= this.VENTANA_RESPUESTA_MS ? 'ESPERANDO' : 'SIN_RESPUESTA';
  },

  // LOS CINCO DESENLACES POSIBLES, y cuatro de ellos son "no lo se" dicho de cuatro
  // maneras distintas. Solo SIN_CAMBIO afirma que no se movio nada, y solo se llega
  // ahi habiendo comparado dos $STATUS de verdad.
  estadoEfecto(reg, ahora) {
    if (!reg || reg.clase !== 'ORDEN') return 'NO_APLICA';
    if (!reg.salio) return 'NO_SALIO';
    const t = typeof ahora === 'number' ? ahora : Date.now();
    const abierta = (t - reg.ms) <= this.VENTANA_EFECTO_MS;
    if (!reg.antes) return 'NO_SE_PUDO_VER';
    if (!reg.statusEnVentana) return abierta ? 'ESPERANDO' : 'NO_SE_PUDO_VER';
    if (Object.keys(reg.cambios).length) return 'CAMBIO';
    // Hubo tramas despues, pero puede que ninguna trajera los campos que se comparan.
    if (!this._comparables(reg).length) return 'NO_SE_PUDO_VER';
    return abierta ? 'ESPERANDO' : 'SIN_CAMBIO';
  },

  // Los campos que estaban en el $STATUS de antes Y en el de despues. Comparar uno que
  // solo aparece en un lado no dice si cambio: dice que dejo de venir, que es otra cosa.
  _comparables(reg) {
    if (!reg.antes || !reg.despues) return [];
    return this.CAMPOS_EFECTO.filter(
      k => reg.antes[k] !== undefined && reg.despues[k] !== undefined);
  },

  // ---- Las tres frases ----------------------------------------------------

  // LA TRAMA SALE ESCAPADA, igual que las de entrada y por el mismo motivo: rawCmd
  // termina en CR LF, y un CR suelto en mitad de una lista la parte y ensena una linea
  // que no es la que salio. Se escapa la REPRESENTACION; en el registro sigue entera.
  textoOrden(reg) {
    if (reg.clase !== 'ORDEN') return '';
    let s = reg.linea === null ? reg.orden : RegistroCrudo.escapar(reg.linea);
    if (!reg.salio) {
      s += '   NO SALIO: ' + reg.motivoNoSalio +
           ' (no llego a escribirse al cable, asi que no esta en la cinta)';
    }
    return s;
  },

  textoRespuesta(reg, ahora) {
    const est = this.estadoRespuesta(reg, ahora);
    if (est === 'NO_SALIO') {
      return 'no hay respuesta que esperar: la orden no salio';
    }
    if (est === 'ESPERANDO') {
      const t = typeof ahora === 'number' ? ahora : Date.now();
      return 'todavia sin respuesta (' + this._seg(t - reg.ms) + ' s; se espera hasta ' +
             this._seg(this.VENTANA_RESPUESTA_MS) + ' s)';
    }
    if (est === 'SIN_RESPUESTA') {
      return 'SIN RESPUESTA: el equipo no contesto en ' +
             this._seg(this.VENTANA_RESPUESTA_MS) + ' s. Eso NO quiere decir que ' +
             'rechazara la orden: quiere decir que esta app no oyo nada';
    }
    const r = reg.respuesta;
    let s = RegistroCrudo.escapar(r.linea) + '   (+' + this._seg(r.ms - reg.ms) + ' s)';
    if (r.atribucion === 'POR_CABECERA') {
      s += '   [el equipo contesta al comando ' + r.cmd + ', sin los argumentos que se mandaron]';
    } else if (r.atribucion === 'POR_ALIAS') {
      s += '   [el equipo contesta al comando ' + r.cmd + ' para una orden que se manda ' +
           'como ' + reg.orden + ': lo hace asi el firmware]';
    } else if (r.atribucion === 'POR_DESCARTE') {
      s += '   [ATRIBUIDA POR DESCARTE: esta trama no nombra la orden a la que ' +
           'contesta, y era la unica esperando. Puede no ser de esta orden]';
    }
    return s;
  },

  textoEfecto(reg, ahora) {
    const est = this.estadoEfecto(reg, ahora);
    const t = typeof ahora === 'number' ? ahora : Date.now();
    if (est === 'NO_SALIO') {
      return 'no hay efecto que mirar: la orden no salio';
    }
    if (est === 'NO_SE_PUDO_VER') {
      if (!reg.antes) {
        return 'NO SE PUDO VER: ' + reg.sinAntes + ', asi que no hay con que comparar';
      }
      if (!reg.statusEnVentana) {
        return 'NO SE PUDO VER: no llego ningun $STATUS en los ' +
               this._seg(this.VENTANA_EFECTO_MS) + ' s siguientes. Sin trama despues no ' +
               'se puede decir si cambio algo';
      }
      return 'NO SE PUDO VER: llegaron ' + reg.statusEnVentana + ' $STATUS despues, pero ' +
             'ninguno traia a la vez con el de antes los campos ' +
             this.CAMPOS_EFECTO.join(' ni ');
    }
    if (est === 'ESPERANDO') {
      // DOS ESPERAS DISTINTAS, Y DECIR "sin cambio" EN LA PRIMERA ES INVENTAR.
      //
      // Lo cazo el propio ejercicio de 8.bis: con la ventana recien abierta y CERO
      // tramas detras, esta frase salia "sin cambio todavia: no hay campos que
      // comparar en los 0 $STATUS de los 0,0 s que van". O sea, afirmando que no
      // habia cambiado nada sobre ningun dato -que es exactamente lo que este fichero
      // existe para no hacer-. Sin trama posterior no hay "sin cambio": hay silencio.
      if (!reg.statusEnVentana) {
        return 'todavia no se puede ver: desde que salio la orden no ha llegado ningun ' +
               '$STATUS (van ' + this._seg(t - reg.ms) + ' s de los ' +
               this._seg(this.VENTANA_EFECTO_MS) + ' s que se miran)';
      }
      return 'sin cambio todavia: ' + this._campoAcampo(reg, true) + ' en los ' +
             reg.statusEnVentana + ' $STATUS de los ' + this._seg(t - reg.ms) +
             ' s que van (se mira hasta ' + this._seg(this.VENTANA_EFECTO_MS) + ' s)';
    }
    if (est === 'CAMBIO') {
      const l = Object.keys(reg.cambios).map(k =>
        k + ': ' + reg.cambios[k].de + ' -> ' + reg.cambios[k].a +
        ' (a los ' + this._seg(reg.cambios[k].ms - reg.ms) + ' s)');
      return 'CAMBIO: ' + l.join(' · ');
    }
    // SIN_CAMBIO. La unica frase que afirma algo, y solo se llega aqui con un $STATUS
    // antes, otro despues y los mismos valores en los dos.
    return 'NO CAMBIO NADA: ' + this._campoAcampo(reg, false) + ' en los ' +
           reg.statusEnVentana + ' $STATUS de los ' + this._seg(this.VENTANA_EFECTO_MS) +
           ' s posteriores';
  },

  _campoAcampo(reg, presente) {
    const campos = this._comparables(reg);
    if (!campos.length) return 'no hay campos que comparar';
    return campos.map(k => k + (presente ? ' sigue en ' : ' siguio en ') + reg.antes[k])
                 .join(' y ');
  },

  // ---- Lectura ------------------------------------------------------------

  // Las mas recientes primero, que es el orden en el que se mira con el equipo delante.
  recientes(limite) {
    const n = typeof limite === 'number' && limite > 0 ? limite : this._diario.length;
    return this._diario.slice(-n).reverse();
  },

  todas() {
    return this._diario.slice();
  },

  // Cuantas ordenes salieron, cuantas se quedaron sin respuesta y cuantas no movieron
  // nada. NO se cuentan aqui las que estan ESPERANDO: una orden de hace dos segundos
  // sin respuesta todavia no es una orden sin respuesta.
  contadores(ahora) {
    const t = typeof ahora === 'number' ? ahora : Date.now();
    const c = {
      ordenes: 0, noSalieron: 0, sinRespuesta: 0, rechazadas: 0,
      sinCambio: 0, noSePudoVer: 0, sueltas: 0, enCurso: 0,
      descartados: this.descartados
    };
    this._diario.forEach(r => {
      if (r.clase === 'RESPUESTA_SUELTA') { c.sueltas++; return; }
      c.ordenes++;
      if (!r.salio) { c.noSalieron++; return; }
      const er = this.estadoRespuesta(r, t);
      if (er === 'SIN_RESPUESTA') c.sinRespuesta++;
      if (er === 'ESPERANDO') c.enCurso++;
      if (er === 'LLEGO' && r.respuesta.tipo === '$ERR') c.rechazadas++;
      const ee = this.estadoEfecto(r, t);
      if (ee === 'SIN_CAMBIO') c.sinCambio++;
      if (ee === 'NO_SE_PUDO_VER') c.noSePudoVer++;
    });
    return c;
  },

  // ---- Exportacion --------------------------------------------------------
  //
  // ESTO ES LO QUE LLEGA POR WHATSAPP y lo lee alguien que no programa. Texto plano,
  // corto, tres lineas por orden y ni una abreviatura que haya que preguntar. La cinta
  // en crudo se exporta aparte: quien quiera los bytes ya sabe donde estan.
  aTexto(ahora, cabecera) {
    const t = typeof ahora === 'number' ? ahora : Date.now();
    const c = this.contadores(t);
    const l = [];
    l.push('IOT-VIAL - DIARIO DE ORDENES');
    if (cabecera) Object.keys(cabecera).forEach(k => l.push(k + ': ' + cabecera[k]));
    l.push('Exportado: ' + new Date(t).toISOString());
    l.push('');
    l.push('QUE ES ESTO: una entrada por cada orden que se dio desde este telefono, con');
    l.push('lo que se mando, lo que el equipo contesto y lo que se vio cambiar despues.');
    l.push('El PIN sale tapado con asteriscos; al equipo se le manda entero.');
    l.push('');
    l.push('LO QUE ESTE DIARIO NO SABE, para que no se lea de mas:');
    l.push('  - No sabe si el equipo RECIBIO la orden, solo que la app la escribio.');
    l.push('  - "NO CAMBIO NADA" significa que MODO y ESTADO no se movieron en las');
    l.push('    tramas que llegaron. No significa que el firmware desobedeciera.');
    l.push('  - No ve las luces del poste: ve lo que el equipo dice por $STATUS.');
    l.push('  - No sabe de ordenes dadas desde otro telefono, el mando o los botones.');
    l.push('');
    l.push('RESUMEN');
    l.push('  ordenes anotadas       : ' + c.ordenes);
    l.push('  no llegaron a salir    : ' + c.noSalieron);
    l.push('  sin respuesta          : ' + c.sinRespuesta);
    l.push('  rechazadas por el equipo: ' + c.rechazadas);
    l.push('  no movieron MODO ni ESTADO: ' + c.sinCambio);
    l.push('  efecto que no se pudo ver : ' + c.noSePudoVer);
    if (c.enCurso) l.push('  todavia en curso       : ' + c.enCurso);
    if (c.sueltas) {
      l.push('  respuestas sueltas     : ' + c.sueltas +
             '  (llegaron sin una orden de esta app a la que atribuirlas)');
    }
    if (c.descartados) {
      l.push('  RECORTADO: se tiraron las ' + c.descartados + ' entradas mas antiguas ' +
             'al llegar al tope de ' + this.TOPE + '. Lo de antes de eso no esta aqui.');
    }
    l.push('');
    l.push('LAS ORDENES, DE LA MAS ANTIGUA A LA MAS NUEVA');
    l.push('');
    if (!this._diario.length) {
      l.push('  (vacio: no se ha dado ninguna orden en esta sesion de la app)');
    }
    this._diario.forEach(r => {
      const h = this._hora(r.ms);
      if (r.clase === 'RESPUESTA_SUELTA') {
        l.push(h + '  RESPUESTA SUELTA  ' + RegistroCrudo.escapar(r.respuesta.linea));
        l.push('          ' + r.respuesta.motivoSuelta);
        l.push('');
        return;
      }
      l.push(h + '  ORDEN      ' + this.textoOrden(r));
      const est = this.estadoRespuesta(r, t);
      const hr = est === 'LLEGO' ? this._hora(r.respuesta.ms) : '        ';
      l.push(hr + '  RESPUESTA  ' + this.textoRespuesta(r, t));
      l.push('          EFECTO     ' + this.textoEfecto(r, t));
      l.push('');
    });
    return l.join('\n') + '\n';
  }
};

if (typeof module !== 'undefined' && module.exports) {
  // RegistroCrudo sigue siendo la exportacion por defecto porque tests/test_unitarios.js
  // lo requiere asi desde antes; el diario se cuelga al lado en vez de cambiar la forma
  // del modulo, que romperia a quien ya lo usa.
  module.exports = RegistroCrudo;
  module.exports.DiarioOrdenes = DiarioOrdenes;
}
