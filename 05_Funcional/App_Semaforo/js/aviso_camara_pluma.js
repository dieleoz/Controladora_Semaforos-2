// ===== js/aviso_camara_pluma.js =====
//
// LA BARRERA QUE LA CAMARA DEJA ARRIBA, Y POR QUE ESO NO ES UNA LINEA DE BITACORA.
//
// EL DATO Y DE DONDE SALE. Desde el 14/09 las dos puntas publican un $EVENT con
// ORIGEN:CAMARA_PLUMA y dos DETALLE distintos, medidos en {Maestro,Esclavo}/src/
// botones.cpp:
//
//   VETO_ACTUADO_N:<n>     en el FLANCO de cada veto. Es lo NORMAL: la camara vio algo
//                          debajo del brazo y la barrera no bajo. Un vehiculo despeja en
//                          segundos y esto se cierra solo. No abre ningun cartel.
//   VETO_SOSTENIDO_S:<s>   la barrera lleva retenida MAS que el todo-rojo mas largo que
//                          el equipo admite, y el firmware lo REPITE mientras dure. Eso
//                          ya no es trafico: es lo que hay que ver.
//
// BASTA UNA VEZ (decision del responsable, 14/09). No hay umbral que construir ni
// alarmas que contar, y por eso aqui no hay ninguna cuenta: el criterio ya es FISICO
// -el firmware lo comparo contra el despeje mas largo antes de publicar- y repetirlo en
// la app seria una segunda copia de una regla que vive en el C++.
//
// 🔴 LO QUE ESTE FICHERO TIENE PROHIBIDO DECIR: «CAMARA AVERIADA».
//
// El equipo NO VE IMAGEN (D-12) y no puede separar un vehiculo parado debajo de una
// camara mal apuntada: los dos son un contacto cerrado, mismo nivel, mismo pin. Lo unico
// MEDIDO son los segundos que lleva retenida, y quien juzga es la persona que esta
// delante del poste. El propio botones.cpp lo deja escrito -«NO dice "camara averiada"
// [...] Quien traduce esto a "revise el ajuste de la camara" es la app»-, asi que esta
// app traduce la ACCION y no inventa el DIAGNOSTICO. Un texto que afirmara la averia
// mandaria a cambiar una camara sana y, peor, a no mirar debajo del brazo.
//
// POR QUE HAY UN CARTEL Y NO SOLO UNA LINEA, QUE ES LO QUE LA APP HACIA HASTA HOY.
//
// addEvent() recorta state.events a 30, y la ventana del POSTE 2 ensena slice(0, 12). El
// aviso llegaba, se pintaba en cyan como una linea de registro cualquiera y se iba con
// el scroll en cuanto el equipo hablaba otras doce veces. Es el mismo defecto que D-23
// destapo con el periodico de la radio, con una diferencia que lo empeora: aquel dato
// tapaba avisos, este ERA el aviso. Un cartel que vive FUERA de state.events no se puede
// desalojar contando lineas, que es lo unico que se le pedia.
//
// Y EL CARTEL VIVE FUERA DE LAS PESTANAS, no dentro de la de ESTADO. Quien mira la
// bitacora o los tiempos tiene la pestana de estado oculta, y un cartel que solo se ve
// en una pestana se pierde igual que una linea: se pierde por navegacion en vez de por
// scroll. Va como primer hijo de .app-container, que es comun a las cinco.
//
// LA CADENCIA NO GASTA BITACORA; LAS TRANSICIONES SI. Misma regla que
// js/diagnostico_enlace.js. El firmware repite el aviso mientras dure la retencion: una
// linea por repeticion volveria a comerse las 30. Se escribe UNA al empezar el episodio
// y UNA al soltarse; entre medias solo se refresca el cartel.
//
// COMO SE SABE QUE YA BAJO, y por que no se adivina. El firmware NO publica un evento
// de fin: deja de repetir, y «dejar de repetir» es indistinguible de «se corto el
// Bluetooth». Quien lo dice es el campo PLUMA del $STATUS, que llega a cadencia fija de
// las dos puntas (N-153). Mientras no llegue un PLUMA:ABAJO, el cartel NO se retira: lo
// que cambia es su titulo, porque «la barrera esta ARRIBA» dejaria de ser cierto y un
// cartel que miente es peor que ninguno.
//
// SIN CIFRAS DEL FIRMWARE (CLAUDE.md 14). El unico numero que este fichero ensena son
// los segundos que VIENEN EN LA TRAMA, que es un dato medido por el equipo y no una
// constante copiada del C++. Ni el todo-rojo maximo, ni la cadencia del aviso, ni el
// tope del buffer se escriben aqui: esta app no puede recalcularlos.
//
// NO PARTE TRAMAS (app_12_un_solo_parser). Aqui llega el objeto YA partido por
// _camposNmea() de app.js y solo se lee el VALOR del campo DETALLE.

const AvisoCamaraPluma = {

  ORIGEN: 'CAMARA_PLUMA',

  // LOS DOS PATRONES VAN ANCLADOS EN LOS DOS EXTREMOS, y no es celo: con este mismo
  // ORIGEN pueden crecer manana otros DETALLE, y un patron que solo mirara el prefijo se
  // los tragaria en el cartel equivocado. Lo que la tabla no nombra sale en crudo por el
  // camino generico de app.js, que es la mitad que impide que esto sea un filtro.
  //
  // EL '!' NO ES UN ERROR DE FORMATO: ES UN VALOR DEL PROTOCOLO. snprintf no avisa de
  // que recorta, asi que el firmware publica "!" -"llego imposible"- en vez de un numero
  // bien formado y falso cuando los segundos no caben en su campo (N-154). Nunca "--",
  // que en este protocolo significa "todavia no lo se". Si el patron no lo admitiera,
  // justo el caso mas grave -la retencion mas larga- caeria al camino del crudo.
  PATRON_SOSTENIDO: /^VETO_SOSTENIDO_S:(\d+|!)$/,
  PATRON_ACTUADO: /^VETO_ACTUADO_N:(\d+)$/,

  // LO UNICO ACCIONABLE, Y EL ORDEN DE LAS TRES FRASES ES EL ORDEN DE LOS RIESGOS.
  // Primero lo que puede matar a alguien -hay un brazo levantado que no va a bajar-,
  // luego mirar debajo ANTES de tocar nada, y solo al final la camara. Al reves mandaria
  // a un tecnico a la caja de la camara con un vehiculo todavia debajo del brazo.
  ACCION: 'La barrera está ARRIBA y no va a bajar sola. Mire debajo del brazo antes de ' +
          'tocar nada. Si no hay nada, revise el apunte y la configuración de la cámara.',

  // Lo que el equipo NO puede saber, dicho para que nadie lea el cartel como un
  // diagnostico. Va SIEMPRE, tambien cuando la barrera ya bajo.
  LIMITE: 'El equipo no ve imagen: no distingue un vehículo parado debajo de una cámara ' +
          'mal apuntada. Lo único que mide son los segundos que lleva retenida. ' +
          'Quien juzga es usted.',

  // ---- Estado de la sesion ----------------------------------------------
  //
  // _cartel es lo que sobrevive a las 30 lineas. Nace null y NO vuelve a null solo: una
  // vez que el equipo ha dicho que tuvo la barrera retenida, eso paso, y borrarlo al
  // bajar la barrera dejaria al tecnico sin el unico rastro de por que vino.
  _cartel: null,
  // Dentro de un episodio el firmware repite; esta bandera es lo que impide que cada
  // repeticion gaste una linea de la bitacora.
  _enEpisodio: false,

  // Al soltar el enlace se olvida TODO, igual que DiagnosticoEnlace: un cartel del poste
  // anterior colgado sobre el poste siguiente manda a mirar debajo del brazo que no es.
  olvidar() {
    this._cartel = null;
    this._enEpisodio = false;
  },

  // ---- Lectura ----------------------------------------------------------

  esDeLaCamara(data) {
    return !!data && data.ORIGEN === this.ORIGEN;
  },

  // Devuelve null si la trama no es de este sujeto -el llamador sigue por su camino de
  // siempre-, o {clase, segundos, fueraDeCota, vetos}.
  //
  // `segundos` es null cuando el equipo publico '!', y entonces fueraDeCota es true. Son
  // dos cosas distintas y no se colapsan en un 0: un 0 diria "lleva cero segundos", que
  // es lo contrario de lo que pasa.
  leer(data) {
    if (!this.esDeLaCamara(data)) return null;
    const det = String(data.DETALLE === undefined || data.DETALLE === null
      ? '' : data.DETALLE).trim();

    const s = this.PATRON_SOSTENIDO.exec(det);
    if (s) {
      const fuera = s[1] === '!';
      return {
        clase: 'SOSTENIDO',
        segundos: fuera ? null : Number(s[1]),
        fueraDeCota: fuera,
        vetos: null
      };
    }
    const a = this.PATRON_ACTUADO.exec(det);
    if (a) {
      return { clase: 'ACTUADO', segundos: null, fueraDeCota: false, vetos: Number(a[1]) };
    }
    return null;
  },

  // ---- El aviso, y lo que de verdad merece una linea --------------------
  //
  // Devuelve null si la trama no es de este sujeto. Si lo es:
  //
  //   clase   'SOSTENIDO' o 'ACTUADO'
  //   abre    este aviso ABRE el cartel (primera retencion sostenida del episodio)
  //   linea   {tono, texto} para la bitacora, o null si esta repeticion no gasta linea
  //   toast   el emergente, solo cuando abre
  ver(data) {
    const leido = this.leer(data);
    if (!leido) return null;

    if (leido.clase === 'ACTUADO') {
      // UN VETO QUE ACTUA ES LA CAMARA HACIENDO SU TRABAJO, y se cuenta como suceso: hay
      // uno por veto, no uno por vuelta del loop. Se traduce para que no se lea como una
      // averia -que es lo que parece en crudo- y NO abre cartel.
      //
      // Y CIERRA EL EPISODIO ANTERIOR PARA LA BITACORA: el flanco de subida de un veto
      // solo ocurre si el anterior se solto, asi que la siguiente retencion sostenida es
      // OTRO episodio y merece su linea. Quien decide si la barrera esta abajo NO es
      // esto -es el campo PLUMA-, y por eso aqui no se toca el cartel.
      this._enEpisodio = false;
      return {
        clase: 'ACTUADO',
        abre: false,
        linea: {
          tono: 'cyan',
          texto: 'La cámara de este poste ha impedido que la barrera baje porque ve algo ' +
                 'debajo del brazo. Es lo normal mientras pasa un vehículo: se suelta solo ' +
                 'cuando deja de verlo.'
        },
        toast: null
      };
    }

    const primera = !this._enEpisodio;
    this._enEpisodio = true;
    this._cartel = {
      node: (data && data.NODE) || null,
      segundos: leido.segundos,
      fueraDeCota: leido.fueraDeCota,
      // La hora del EQUIPO, no la del telefono: es la que hay que citar al reportar.
      // Puede venir '--:--:--' si ese poste no esta en hora.
      hora: (data && data.HORA) || null,
      bajada: false
    };

    return {
      clase: 'SOSTENIDO',
      abre: primera,
      // Solo la PRIMERA del episodio gasta bitacora. Las repeticiones refrescan el
      // cartel y se callan, que es lo que impide que este aviso se coma las 30 lineas.
      linea: primera ? { tono: 'red', texto: this.textoLinea() } : null,
      toast: primera ? 'Barrera retenida por la cámara - mire debajo del brazo' : null
    };
  },

  // EL $STATUS ES QUIEN DICE QUE YA BAJO. Devuelve la linea de bitacora que corresponde
  // -una sola, al soltarse-, o null. Un PLUMA que no vino (null) NO cierra nada: "no
  // tengo el dato" no es "la barrera bajo", y confundirlos retiraria el aviso por un
  // equipo con firmware anterior a N-153.
  verPluma(valor) {
    if (!this._cartel || this._cartel.bajada) return null;
    if (valor !== 'ABAJO') return null;
    this._cartel.bajada = true;
    this._enEpisodio = false;
    return {
      tono: 'green',
      texto: 'La barrera de este poste ya ha bajado: la cámara ha dejado de retenerla. ' +
             'El aviso se queda en pantalla porque lo que la dejó arriba sigue sin revisarse.'
    };
  },

  // Lo que hay que pintar, o null si no hay cartel. Quien pinta compone el DOM; aqui se
  // devuelve el dato y las frases, que es lo que las suites pueden medir sin navegador.
  vigente() {
    if (!this._cartel) return null;
    const c = this._cartel;
    return {
      node: c.node,
      segundos: c.segundos,
      fueraDeCota: c.fueraDeCota,
      hora: c.hora,
      bajada: c.bajada,
      titulo: c.bajada
        ? 'LA BARRERA YA HA BAJADO — el aviso queda por lo que la dejó arriba'
        : 'LA CÁMARA TIENE LA BARRERA RETENIDA',
      // La ACCION solo se da mientras el brazo esta arriba: mandar a mirar debajo de una
      // barrera que ya bajo es mandar a mirar donde no hay nada, y el tecnico deja de
      // creerse el cartel la proxima vez.
      accion: c.bajada
        ? 'Antes de irse, revise el apunte y la configuración de la cámara: lo que dejó ' +
          'la barrera arriba no se ha tocado, y puede volver a pasar.'
        : this.ACCION,
      limite: this.LIMITE,
      medida: this.textoMedida()
    };
  },

  // LA MEDIDA, QUE ES LO UNICO QUE EL EQUIPO SABE. Se escribe aparte del titulo porque
  // es lo que cambia en cada repeticion y lo unico que se puede citar al reportar.
  textoMedida() {
    const c = this._cartel;
    if (!c) return '';
    const quien = c.node === 'MAESTRO' ? 'POSTE 1 (MAESTRO)'
                : c.node === 'ESCLAVO' ? 'POSTE 2 (ESCLAVO)'
                // Sin NODE no se adivina: atribuir el aviso al poste equivocado manda a
                // abrir el gabinete que no es. Misma regla que _cual() de avisos_equipo.
                : 'este poste (la trama no dice cuál)';
    // EL TIEMPO VERBAL SIGUE AL BRAZO. Una barrera que ya bajo y un cartel que dice
    // "lleva retenida" son la misma clase de mentira que un rotulo que sobrevive a su
    // sujeto (CLAUDE.md 14): el que lo lee sale a mirar un brazo que ya no esta arriba.
    const lleva = c.bajada ? 'estuvo retenida' : 'lleva retenida';
    const cuanto = c.fueraDeCota
      // '!' es el propio equipo diciendo que el numero no le cabe. Se dice asi, sin
      // inventar una cifra y sin callarlo: es el caso mas grave, no el mas dudoso.
      ? lleva + ' más tiempo del que el equipo puede publicar en el aviso'
      : lleva + ' ' + c.segundos + ' s';
    return quien + ': ' + cuanto +
           (c.hora ? ', según el reloj del equipo a las ' + c.hora : '') + '.';
  },

  // La linea que va a la bitacora al abrir el episodio. Lleva la medida y la accion: la
  // bitacora se exporta y se reporta, y una linea que solo dijera "barrera retenida" no
  // le serviria de nada a quien lea el informe una semana despues.
  textoLinea() {
    return 'BARRERA RETENIDA POR LA CÁMARA. ' + this.textoMedida() + ' ' + this.ACCION;
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = AvisoCamaraPluma;
}
