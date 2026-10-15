// ===== js/diagnostico_enlace.js =====
//
// COMO VE ESTE POSTE SU PROPIA RADIO. LA MITAD DE D-23 QUE FALTABA EN LA APP.
//
// EL DATO Y DE DONDE SALE. Desde el 13/09 el Esclavo publica cada DIAG_ENLACE_MS un
// $EVENT con ORIGEN:ENLACE_RF y DETALLE:RX:<n> OK:<n> RUIDO:<n> -- los tres contadores
// libres de protocolo.cpp, vistos DESDE LA PUNTA A LA QUE ESTAS CONECTADO:
//
//   RX     bytes que entraron por la linea de la radio
//   OK     tramas que pasaron el CRC
//   RUIDO  tramas que entraron y se descartaron
//
// POR QUE ESTO NO ES UNA LINEA MAS DE LA BITACORA, QUE ES LO QUE HACIA LA APP HASTA HOY.
//
// Hasta el 13/09 los mismos tres contadores SOLO salian dentro del $ALARM, o sea cuando
// el enlace YA se habia caido. D-23 los saca a cadencia fija para poder ver venir la
// caida. Pero el $EVENT aterrizaba en addEvent(), y addEvent() RECORTA state.events A
// 30: un periodico cada 30 s son 2 por minuto, y en QUINCE MINUTOS la lista entera es
// este dato y no queda un solo $ALARM de los que el tecnico vino a leer.
//
// 🔴 Y EN LA VENTANA DONDE MAS DUELE SON SEIS MINUTOS, NO QUINCE: renderDiagnostico()
// -- la ventana del POSTE 2, la unica que se abre con el poste delante -- ensena
// state.events.slice(0, 12). Doce entradas a dos por minuto son SEIS MINUTOS hasta que
// el periodico se ha comido entera la pantalla que existe para diagnosticar. El dato
// que venia a ayudar a ver la averia tapaba la averia.
//
// POR QUE TAMPOCO VA A RegistroEnlace, QUE ERA EL CANDIDATO NATURAL Y ESTA MEDIDO.
//
//   1. LE ROMPE EL HORIZONTE QUE EL MISMO PUBLICA. RegistroEnlace.horizonteHoras() se
//      CALCULA -- TOPE * PERIODO_MUESTRA_MS -- y hoy da 6,67 h. Ese calculo supone que
//      quien escribe a cadencia fija es la MUESTRA de rutina, 1/min. Metiendo aqui 2/min
//      mas, el registro se llena en 400/3 = 133 min = 2,2 h mientras la pantalla sigue
//      prometiendo 6,67: la funcion no puede ver un tercer escritor, asi que no
//      protestaria -- mentiria --. Es justo lo que su comentario dice que no debe pasar.
//   2. LA COLUMNA DEL ENLACE SALDRIA VACIA SIEMPRE. RegistroEnlace guarda rf/rtt, y
//      contra el ESCLAVO el $STATUS manda RF:-- y RTT:-- fijos: enlaceDeAhora() devuelve
//      no-medido y cada fila seria un null. Serian 2 filas por minuto SIN el dato que
//      esa tabla existe para guardar, desalojando a las que si lo tienen.
//   3. NO ES LA MISMA MAGNITUD. Alli se guarda el % de latidos contestados que mide el
//      MAESTRO sobre el cruce; esto son los contadores de linea de UNA punta. Mezclarlas
//      en una columna es invitar a restar una de otra.
//
// LO QUE SI VA A LA BITACORA, Y ES LA REGLA DE ESTE FICHERO: LAS TRANSICIONES, NO EL
// RELOJ. Un periodico anota cuando algo CAMBIA -- empieza a entrar ruido, deja de
// entrar, el poste se queda mudo, los contadores se reinician --, y entonces la cuenta
// de lineas la gobiernan los sucesos y no la cadencia. Una muestra de rutina que dice lo
// mismo que la anterior no gasta una linea de las 30: repinta el panel y se calla.
//
// 🔴 LO QUE ESTE FICHERO TIENE PROHIBIDO: JUZGAR. No hay aqui un "RUIDO > x% = malo", y
// no es olvido. El firmware ya tomo esa decision y la dejo escrita en Maestro
// bluetooth.cpp: "Poner aqui un RF < 70% = degradado seria una constante que nadie ha
// decidido gobernando lo que el tecnico ve, asi que el numero VIAJA EN EL DETALLE y el
// juicio lo hace quien lo lee". Un umbral inventado en la app seria esa misma constante
// entrando por la puerta de atras, y ademas decidiendo sobre una radio que esta app no
// ha medido nunca. Se publican los numeros y sus diferencias; el juicio es del tecnico.
//
// TAMPOCO PARTE TRAMAS. app_12_un_solo_parser vigila que la operacion de partir una
// trama en campos este escrita UNA vez: esa es _camposNmea() de app.js y sigue siendo la
// unica. Aqui llega el objeto YA PARTIDO y solo se lee el VALOR del campo DETALLE, que
// es otra operacion. Si algun dia esto empieza a ver `line` en crudo, esta violando ese
// pack.

const DiagnosticoEnlace = {

  // El ORIGEN es el mismo que el del $EVENT de la vuelta del enlace, a proposito: el
  // firmware lo dice en Esclavo/src/bluetooth.cpp -- "es el MISMO sujeto, y la app ya los
  // distingue por el DETALLE" --. De ahi que aqui no baste con mirar ORIGEN.
  ORIGEN: 'ENLACE_RF',

  // SOLO EL PERIODICO, Y POR ESO EL PATRON VA ANCLADO EN LOS DOS EXTREMOS. Con
  // ORIGEN:ENLACE_RF viajan HOY tres formas distintas, medidas en el fuente:
  //
  //   Esclavo periodico (D-23)   "RX:12 OK:3 RUIDO:1"          <- esta, y solo esta
  //   Esclavo vuelta del enlace  "RECUPERADO_OK:3_RUIDO:1"     <- suceso: sigue a eventos
  //   Maestro cambio de estado   "PERDIDO_RF:80" / "OK_RF:--"  <- suceso: sigue a eventos
  //
  // Las dos ultimas son CAMBIOS DE ESTADO, no cadencia: salen una vez por suceso y
  // pertenecen a la bitacora. Sacarlas de ahi por parecerse en el ORIGEN seria esconder
  // justo las lineas que cuentan una caida. Un patron sin anclar -- o que solo mirara
  // "RX:" -- se las llevaria por delante el dia que a un DETALLE le crezca un prefijo.
  //
  // D-60/D-64: el firmware nuevo anade REIN:<r> (arranques contados, da la vuelta en 255); el
  // que esta en campo no lo lleva, asi que es OPCIONAL y su ausencia se lee como null.
  // D-92: el Maestro anade AMBAR:<o> solo mientras esta en MODO_AMBAR; tambien OPCIONAL.
  PATRON: /^RX:(\d+)\s+OK:(\d+)\s+RUIDO:(\d+)(?:\s+REIN:(\d+))?(?:\s+AMBAR:(APP|POSTE2|ARRANQUE))?$/,

  // D-93 (SPEC_4 9.11): el origen se afirma solo cuando se ha leido; la clave '' es "sin dato".
  FRASES_AMBAR: {
    '': 'ámbar en este poste (origen sin dato): pasar con precaución',
    APP: 'ámbar pedido por el operario: pasar con precaución',
    POSTE2: 'ámbar pedido desde el POSTE 2: pasar con precaución',
    ARRANQUE: 'ámbar de arranque (poste reiniciado): pasar con precaución'
  },

  // D-64: la causa del acuse de REINICIOS, en palabras (SPEC_4 9.4).
  CAUSAS: {
    WATCHDOG: 'el vigilante lo reinició: el firmware se quedó colgado (SFTY-1)',
    CORTE: 'se fue la luz o se puso la pila',
    SOFTWARE: 'reinicio pedido por el firmware',
    PIN: 'pata NRST: carga del firmware por SWD o reset a mano',
    OTRA: 'otra causa'
  },

  // ---- Estado de la sesion ----------------------------------------------
  //
  // Los contadores son LIBRES y arrancan en cero con el micro, asi que solo significan
  // algo comparados con la muestra ANTERIOR DE ESTE MISMO POSTE. Al soltar el enlace se
  // olvidan: restar la muestra de un poste contra la del otro fabricaria un delta
  // inventado, y ademas negativo, que es como se lee un reinicio.
  _ultima: null,
  // El delta se GUARDA en vez de recalcularse al pintar: pintar ocurre tambien cuando no
  // ha llegado nada -al abrir la ventana del poste-, y recalcularlo alli obligaria a
  // guardar la penultima muestra solo para eso. Aqui hay un sitio y uno solo.
  _ultimoDelta: null,
  _mudo: false,
  _ruidoso: false,
  // D-64: REINICIOS se pide sola una vez por sesion con el primer REIN, y a cada cambio.
  _reinPedido: false,
  _reiniciosEnCurso: false,
  _ambar: null,   // D-93: ultimo AMBAR: leido; se olvida con un MODO distinto de AMBAR

  olvidarAmbar() { this._ambar = null; },
  fraseAmbar() { return this.FRASES_AMBAR[this._ambar || '']; },

  olvidar() {
    this._ambar = null;
    this._ultima = null;
    this._ultimoDelta = null;
    this._mudo = false;
    this._ruidoso = false;
    this._reinPedido = false;
    this._reiniciosEnCurso = false;
  },

  // ---- Lectura ----------------------------------------------------------

  esPeriodico(data) {
    if (!data || data.ORIGEN !== this.ORIGEN) return false;
    return this.PATRON.test(String(data.DETALLE === undefined || data.DETALLE === null
      ? '' : data.DETALLE).trim());
  },

  leer(data) {
    if (!this.esPeriodico(data)) return null;
    const m = this.PATRON.exec(String(data.DETALLE).trim());
    // Number() y no parseInt(): el tope de los tres es el de un unsigned long del C++
    // (4.294.967.295) y un double lo representa exacto, asi que no hay redondeo que
    // explicar. parseInt aceptaria ademas basura detras de las cifras, y el patron ya
    // dijo que no la hay.
    return { rx: Number(m[1]), ok: Number(m[2]), ruido: Number(m[3]),
             rein: m[4] === undefined ? null : Number(m[4]), ambar: m[5] || null };
  },

  // ---- La muestra, su delta y lo que merece una linea --------------------
  //
  // Devuelve null si la trama no es el periodico -- el llamador debe seguir por su
  // camino de siempre --. Si lo es, devuelve:
  //
  //   muestra   los tres contadores tal y como vinieron, con su instante
  //   delta     lo que cambio DESDE LA MUESTRA ANTERIOR, o null si no hay con que
  //             comparar (primera del poste, o los contadores se reiniciaron)
  //   primera   es la primera muestra de este poste en esta sesion
  //   reinicio  algun contador BAJO, o sea que el poste arranco de nuevo
  //   avisos    las lineas que SI gastan sitio en la bitacora, ya redactadas
  //
  // `ms` se admite por parametro para poder probar dos muestras seguidas sin esperar
  // treinta segundos, igual que hace RegistroEnlace.anotar().
  ver(data, ms) {
    const leido = this.leer(data);
    if (!leido) return null;
    const ahora = typeof ms === 'number' ? ms : Date.now();
    const ant = this._ultima;
    const muestra = {
      rx: leido.rx, ok: leido.ok, ruido: leido.ruido, ms: ahora, rein: leido.rein,
      nodo: data.NODE || null,
      // La hora del EQUIPO, que no tiene por que ser la del telefono y es la que hay que
      // citar al reportar. Puede venir "--:--:--" si ese poste no esta en hora.
      hora: data.HORA || null
    };

    const primera = !ant;
    // UN CONTADOR QUE BAJA NO ES UN DELTA NEGATIVO, pero tampoco prueba un arranque: el
    // 06/10 lo puso a cero PRUEBA ALCANCE (SFTY-15, retirado por D-63). D-64: reinicio es
    // solo que REIN CAMBIE en el mismo NODE; la bajada sin REIN distinto se dice aparte.
    const bajada = !!ant && (leido.rx < ant.rx || leido.ok < ant.ok ||
                             leido.ruido < ant.ruido);
    const conRein = !!ant && ant.rein !== null && leido.rein !== null &&
                    ant.nodo === muestra.nodo;
    const reinicio = conRein && leido.rein !== ant.rein;
    const pedirReinicios = leido.rein !== null && (!this._reinPedido || reinicio);
    if (pedirReinicios) { this._reinPedido = true; this._reiniciosEnCurso = true; }

    let delta = null;
    if (ant && !bajada && !reinicio) {
      delta = {
        rx: leido.rx - ant.rx,
        ok: leido.ok - ant.ok,
        ruido: leido.ruido - ant.ruido,
        ventanaMs: ahora - ant.ms
      };
    }

    const avisos = [];

    if (primera) {
      avisos.push({
        tono: 'cyan',
        texto: 'Diagnóstico del enlace de este poste (D-23): la radio lleva ' +
               leido.rx + ' bytes recibidos, ' + leido.ok + ' tramas buenas y ' +
               leido.ruido + ' descartadas desde que arrancó. A partir de aquí solo se ' +
               'anota lo que cambie; el detalle al día está en la ventana de diagnóstico.'
      });
    }

    if (reinicio || bajada) {
      // El tono es el de una alarma y no el de una nota: un poste que rearranca solo en
      // mitad de un turno es lo que el tecnico ha venido a cazar.
      const bajo = ' (' + ant.rx + '→' + leido.rx + ' bytes)';
      avisos.push({
        tono: 'red',
        texto: reinicio
          ? 'ATENCIÓN: este poste SE HA REINICIADO (REIN ' + ant.rein + '→' + leido.rein +
            ') desde la muestra anterior; se pide la causa al equipo (REINICIOS). La cuenta ' +
            'arranca otra vez aquí.'
          : conRein
            ? 'ATENCIÓN: los contadores de radio de este poste BAJARON SIN REINICIO' + bajo +
              ' (REIN:' + leido.rein + ' sigue): o el contador dio la vuelta en 4294967295 o es ' +
              'un defecto del firmware; repórtelo con esta línea.'
            : 'ATENCIÓN: los contadores de radio de este poste BAJARON' + bajo + ': o se ' +
              'reinició o entró en PRUEBA ALCANCE; este firmware no publica REIN. La cuenta ' +
              'arranca otra vez aquí.'
      });
      this._mudo = false;
      this._ruidoso = false;
    }

    // LAS DOS TRANSICIONES, QUE SON LO UNICO QUE SE ANOTA EN REGIMEN. Se comparan contra
    // la ventana ANTERIOR, no contra un umbral: se anota el dia que empieza y el dia que
    // para, y entre medias el panel lleva la cuenta sin gastar bitacora. Un poste en un
    // sitio ruidoso no puede generar mas de dos lineas por episodio.
    if (delta) {
      const mudoAhora = delta.rx === 0;
      if (mudoAhora !== this._mudo) {
        this._mudo = mudoAhora;
        avisos.push(mudoAhora ? {
          tono: 'red',
          texto: 'La radio de este poste NO recibió un solo byte en los últimos ' +
                 Math.round(delta.ventanaMs / 1000) + ' s. No es ruido ni CRC malo: no ' +
                 'está entrando nada por la línea.'
        } : {
          tono: 'green',
          texto: 'Vuelven a entrar bytes por la radio de este poste (' + delta.rx +
                 ' en los últimos ' + Math.round(delta.ventanaMs / 1000) + ' s).'
        });
      }

      const ruidosoAhora = delta.ruido > 0;
      if (ruidosoAhora !== this._ruidoso) {
        this._ruidoso = ruidosoAhora;
        avisos.push(ruidosoAhora ? {
          tono: 'amber',
          texto: 'Este poste EMPIEZA a descartar tramas: ' + delta.ruido +
                 ' descartada(s) frente a ' + delta.ok + ' buena(s) en los últimos ' +
                 Math.round(delta.ventanaMs / 1000) + ' s. Entra señal y no se entiende.'
        } : {
          tono: 'green',
          texto: 'Este poste deja de descartar tramas: ninguna descartada en los ' +
                 'últimos ' + Math.round(delta.ventanaMs / 1000) + ' s.'
        });
      }
    }

    // D-93: una linea por CAMBIO de origen, nunca por repeticion del periodico.
    const ambarCambio = leido.ambar !== null && leido.ambar !== this._ambar;
    if (leido.ambar !== null) this._ambar = leido.ambar;
    if (ambarCambio) avisos.push({ tono: 'amber', texto: 'ámbar de este poste: ' + this.fraseAmbar() });

    this._ultima = muestra;
    this._ultimoDelta = delta;
    return { muestra, delta, primera, reinicio, bajada, pedirReinicios, avisos, ambarCambio };
  },

  // D-64: el acuse de REINICIOS (o su rechazo) en una linea, o null si no es suyo. La hora
  // del arranque la pone el TELEFONO (ms - UP): el poste puede no tener hora (SPEC_7).
  acuseReinicios(data, ms) {
    if (!data) return null;
    if (data.CMD === 'REINICIOS' && data.RESULT === 'OK') {
      this._reiniciosEnCurso = false;
      const up = Number(data.UP), antes = Number(data.ANTES);
      const t = new Date((typeof ms === 'number' ? ms : Date.now()) - up * 1000);
      const dd = n => String(n).padStart(2, '0');
      const hace = Math.floor(up / 3600) + ' h ' + Math.floor((up % 3600) / 60) + ' min';
      return { tono: data.CAUSA === 'WATCHDOG' ? 'red' : 'cyan', toast: 'Reinicios consultados',
        texto: 'Reinicios del ' + (data.NODE || 'poste') + ': último arranque el ' +
          dd(t.getDate()) + '/' + dd(t.getMonth() + 1) + ' a las ' + dd(t.getHours()) + ':' +
          dd(t.getMinutes()) + ' (hace ' + hace + '); causa ' + data.CAUSA + ', ' +
          (this.CAUSAS[data.CAUSA] || this.CAUSAS.OTRA) + '; la vida anterior duró ' +
          (antes >= 255 ? 'más de 10 días' : antes + ' h') + '; arranques contados: ' +
          data.REIN + ' (da la vuelta en 255).' };
    }
    const noLaLleva = this._reiniciosEnCurso && data.CMD === 'DESCONOCIDO' &&
                      data.DESC === 'COMANDO_NO_SOPORTADO' && data.NODE !== 'PUENTE';
    if (data.CMD !== 'REINICIOS' && !noLaLleva) return null;
    this._reiniciosEnCurso = false;
    return noLaLleva
      ? { texto: 'Este firmware no lleva REINICIOS.', toast: 'REINICIOS: no está en este firmware' }
      : { texto: 'El equipo no pudo componer la respuesta de REINICIOS (' + (data.DESC || '?') +
                 '): repórtelo.', toast: 'REINICIOS rechazado' };
  },

  // ---- Derivadas para pintar --------------------------------------------

  // Cuantas de las tramas que ENTRARON en la ventana se descartaron. No es un veredicto
  // ni lleva umbral: es la division de dos numeros que ya vienen en la trama.
  //
  // 🔴 Y DEVUELVE null CUANDO NO ENTRO NINGUNA, NUNCA 0. Un 0 % aqui diria "entraron
  // tramas y todas estaban bien", que es el mejor resultado posible; "no entro ninguna"
  // es lo contrario y no puede compartir casilla. Es la misma regla que RegistroEnlace se
  // escribio para la columna del enlace, y la misma que separa ABORTADO de PASS.
  pctRuido(delta) {
    if (!delta) return null;
    const entraron = delta.ok + delta.ruido;
    if (entraron <= 0) return null;
    return Math.round((delta.ruido / entraron) * 100);
  },

  // Lo que hay que pintar ahora mismo, o null si este poste todavia no ha mandado nada.
  // Que no haya muestra NO es un cero: es que no ha llegado, y el que pinta lo dice.
  //
  // `delta` puede ser null CON muestra presente, y son dos "no lo se" distintos que el
  // que pinta tiene que separar: no hay muestra (no ha llegado nada) contra hay muestra
  // pero es la primera (no hay con que compararla todavia).
  resumen() {
    return this._ultima ? { muestra: this._ultima, delta: this._ultimoDelta } : null;
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = DiagnosticoEnlace;
}
