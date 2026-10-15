// ===== js/luces.js =====
// La pintura de las luces: las dos columnas de postes, la frase del centro, el badge de modo, la
// pluma y las camaras. Sale de app.js el 15/10 en un corte SIN cambio de comportamiento (SPEC_4
// 9.12 tanda 2, D-98): el texto es el mismo, linea por linea, con dos espacios mas de sangria.
// Las tablas que consulta viven en js/luces_tablas.js. Lo que necesita del cierre de app.js se lo
// pasa app.js al instalarlo, cada cosa por su nombre:
//
//   state                       el objeto de estado de app.js
//   s1Red..s2Green, s1Text, s2Text  las lamparas y rotulos de las dos columnas
//   phaseDescEl, plumaEl, badgeModoEl, camEstadoEl, camDetalleEl  los demas elementos que pinta
//
// Devuelve { renderLights, pintarPluma, pintarCamaras }; app.js conserva las tres con su nombre
// como envoltorio, porque las llaman la rama de $STATUS, el enlace y el arranque.

const Luces = {
  instalar(dep) {
    const state = dep.state;
    const s1Red = dep.s1Red;
    const s1Amber = dep.s1Amber;
    const s1Green = dep.s1Green;
    const s1Text = dep.s1Text;
    const s2Red = dep.s2Red;
    const s2Amber = dep.s2Amber;
    const s2Green = dep.s2Green;
    const s2Text = dep.s2Text;
    const phaseDescEl = dep.phaseDescEl;
    const plumaEl = dep.plumaEl;
    const badgeModoEl = dep.badgeModoEl;
    const camEstadoEl = dep.camEstadoEl;
    const camDetalleEl = dep.camDetalleEl;

    // LA UNICA FUNCION QUE DECIDE SI 'FALLO COM' SE LEE COMO AVERIA O COMO PEDIDO.
    // Otro MODO (DEGRADADO, RENDIDO, sin dato...) es radio caida o Degradado vencido: frase
    // original. Con AMBAR, D-93: la frase por el ORIGEN leido del periodico (SPEC_4 9.11).
    function _fraseDeEstadoLuces(estado) {
      const info = ESTADOS[estado];
      if (!info) return null;
      if (estado === 'FALLO COM' && state.modo === 'AMBAR') {
        return typeof DiagnosticoEnlace !== 'undefined' ? DiagnosticoEnlace.fraseAmbar()
          : 'ámbar en este poste (origen sin dato): pasar con precaución';
      }
      return info.frase;
    }

    function pintarBadgeModo() {
      if (!badgeModoEl) return;
      const info = MODOS[state.modo];
      badgeModoEl.className = 'badge badge-auto';
      if (info) {
        badgeModoEl.style.background = info.fondo;
        badgeModoEl.style.borderColor = info.borde;
        badgeModoEl.style.color = info.color;
        const est = state.estadoLuces == null ? '' : state.estadoLuces;
        badgeModoEl.textContent = (info.porEstado && info.porEstado[est]) || info.texto;
      } else {
        // Un MODO que esta tabla no conoce se DICE, con el literal a la vista. Callarlo
        // dejaria el badge anterior en pantalla; ensenarlo hace visible el dia que el
        // firmware estrene un modo y a esta lista se le olvide crecer.
        badgeModoEl.style.background = 'rgba(148,163,184,0.15)';
        badgeModoEl.style.borderColor = 'var(--text-muted)';
        badgeModoEl.style.color = 'var(--text-muted)';
        badgeModoEl.textContent = state.modo
          ? '❔ MODO NO RECONOCIDO: ' + state.modo
          : 'SIN DATOS DE MODO';
      }
    }

    // LO QUE SIGNIFICA CADA PAREJA (luz, pluma), QUE ES LO QUE DE VERDAD SE LEE.
    //
    // La pluma sola no dice si lo que pasa es normal: ARRIBA con verde es el ciclo y no
    // hay nada que explicar; ARRIBA sin verde es justo lo que hay que explicar. Se decide
    // con el ESTADO que ya esta en pantalla y no con una segunda copia de la regla del
    // firmware -la regla vive en semaforo.cpp, SFTY-28, y aqui solo se LEE el resultado-.
    //
    // LAS FRASES SON CORTAS Y ESTA MEDIDO POR QUE. A 320 px esta fila da unos 54
    // caracteres por linea; la mas larga de aqui son dos lineas -unos 30 px- y el hueco
    // que hay entre el ultimo mando y la barra de pestanas en esa pantalla son 79 px.
    // Una explicacion de cinco lineas taparia un mando, que es el defecto que la foto de
    // campo del 04/09 destapo. Lo que no se recorta es la palabra que importa: AVERIA.
    function _fraseDeLaPareja(pluma) {
      if (pluma === 'ARRIBA') {
        if (state.estadoLuces === 'VERDE') return 'acompaña al verde';
        if (/^(AMBAR|ROJO)_INT$/.test(state.estadoLuces)) return 'modo noche: arriba en los dos postes';
        // SFTY-6: el ambar intermitente sube la pluma A PROPOSITO -politica del cliente,
        // 27/08-. Se dice aqui aunque la tabla ESTADOS ya lo diga, porque quien mira la
        // barrera puede no estar mirando la lampara.
        //
        // Con MODO:AMBAR (28/09) el ambar lo pidio el operario: "sin enlace" seria una averia que no existe.
        if (state.estadoLuces === 'FALLO COM') {
          return state.modo === 'AMBAR'
            ? 'ámbar pedido por el operario: se pasa con precaución'
            : 'sin radio entre postes: se pasa con precaución';
        }
        // D-45 (SPEC_4 6 (2)): el amarillo es el CIERRE de cada verde, y la pluma sigue arriba
        // en el por diseno (SFTY-28), no por un veto: baja al llegar el rojo.
        if (state.estadoLuces === 'AMARILLO') return 'amarillo de cierre: baja al pasar a rojo';
        if (state.estadoLuces === 'ROJO' || state.estadoLuces === 'ROJO+AMAR') {  // D-53: la pluma sube con el VERDE
          // D-13. Con las camaras habra ratos de LUZ ROJA CON LA PLUMA ARRIBA: hay algo
          // debajo y la barrera no baja. Hoy un operario eso lo lee como averia y llama.
          // La frase lo DICE, no lo insinua.
          return 'con la luz en ROJO · NO es avería: no baja mientras haya alguien debajo';
        }
        return null;
      }
      // Verde con la barrera abajo es la direccion SEGURA y esta admitida en SFTY-28 -la
      // barrera puede ser mas restrictiva que la lampara-, pero el que espera delante
      // necesita saber que la luz no basta.
      if (pluma === 'ABAJO' && state.estadoLuces === 'VERDE') {
        return 'verde, pero espere a que suba';
      }
      return null;
    }

    function pintarPluma() {
      if (!plumaEl) return;
      const decir = (txt, color, fondo) => {
        plumaEl.hidden = false;
        plumaEl.textContent = txt;
        plumaEl.style.color = color || 'var(--text-muted)';
        plumaEl.style.borderColor = color || 'var(--text-muted)';
        plumaEl.style.background = fondo || 'transparent';
      };

      // SIN PUNTA IDENTIFICADA NO HAY POSTE AL QUE ATRIBUIR UNA BARRERA, y la fila se
      // OCULTA en vez de escribir "sin datos". No es cosmetica y el numero esta medido: en
      // esa pantalla -la de antes del primer $STATUS, con los DOS mandos de emergencia a
      // la vista- el ultimo mando diario termina a 493 px y la barra de pestanas empieza a
      // 497 px en un telefono de 320 px. Cuatro pixeles. Una fila mas ahi tapa un mando,
      // que es el defecto que la foto de campo del 04/09 destapo. La ausencia ya la
      // declara el tablero entero: "SIN ENLACE - sin datos del equipo".
      if (!state.node) { plumaEl.hidden = true; return; }

      const rotulo = state.node === 'ESCLAVO' ? 'POSTE 2' : 'POSTE 1';

      // El enlace caido va ANTES que el valor: lo guardado es de hace rato, y una barrera
      // de hace rato no es una barrera. Un "PLUMA ARRIBA" viejo tiene la misma cara que
      // uno de ahora y quien lo lee cruza por debajo.
      if (!state.telemetriaViva) {
        decir(rotulo + ' · PLUMA: sin enlace, el dato ya no vale');
        return;
      }

      // El campo NO VINO: equipo con firmware anterior a N-153. Se dice CUAL es la
      // carencia y no "sin datos" a secas, por lo mismo que en pintarAjeno(): un equipo
      // viejo y una radio caida mandan al tecnico a sitios opuestos.
      //
      // Y "no vino" es SOLO null/undefined. Un PLUMA con el valor vacio SI vino y cae
      // abajo, en el camino del literal desconocido, para que se vea.
      if (state.pluma === null || state.pluma === undefined) {
        decir(rotulo + ' · PLUMA: este equipo no la publica (firmware anterior)');
        return;
      }

      const texto = PLUMA_LEYENDA[state.pluma];
      if (!texto) {
        // Un literal que esta tabla no conoce se ENSENA, igual que hace pintarBadgeModo()
        // con un MODO nuevo. Callarlo dejaria la fila en blanco el dia que el firmware
        // estrene un valor, y una fila en blanco no se distingue de una app vieja.
        decir(rotulo + ' · PLUMA NO RECONOCIDA: "' + state.pluma + '"');
        return;
      }

      const frase = _fraseDeLaPareja(state.pluma);
      // EL COLOR NO ES EL DE LA LUZ, Y ESO ES DELIBERADO. Verde aqui significaria "puede
      // pasar", y la pareja rojo+arriba no autoriza a nadie: quien autoriza es el
      // semaforo. Arriba con verde va en verde; arriba sin verde, en ambar -hay algo que
      // explicar-; abajo, neutro, que es lo normal.
      const color = state.pluma === 'ABAJO' ? 'var(--text-muted)'
                  : (state.estadoLuces === 'VERDE' ? 'var(--green-lamp)' : 'var(--amber-lamp)');
      const fondo = (state.pluma === 'ARRIBA' && state.estadoLuces !== 'VERDE')
                  ? 'rgba(255,179,0,0.12)' : 'transparent';
      decir(rotulo + ' · ' + texto + (frase ? ' · ' + frase : ''), color, fondo);
    }

    function pintarCamaras() {
      if (!camEstadoEl && !camDetalleEl) return;
      const decir = (valor, frase, color) => {
        if (camEstadoEl) {
          camEstadoEl.textContent = valor;
          camEstadoEl.style.color = color || 'var(--text-muted)';
        }
        if (camDetalleEl) camDetalleEl.textContent = frase;
      };

      // El enlace caido va PRIMERO: lo guardado es de hace rato, y una camara que estaba
      // bien hace diez minutos no dice nada de la de ahora. Se retira igual que la hora y
      // la pluma, y con el marcador de ausencia de esta app, que es '--'.
      if (!state.telemetriaViva) {
        decir('--', 'Sin enlace: el dato ya no vale');
        return;
      }

      // El campo NO VINO: equipo con firmware anterior a D-13. Se dice CUAL es la
      // carencia y no "sin datos" a secas, por lo mismo que en la pluma: un equipo viejo
      // y una radio caida mandan al tecnico a sitios opuestos.
      if (state.cam === null || state.cam === undefined) {
        decir('--', 'Este equipo no las publica (firmware anterior)');
        return;
      }

      // D-24, segunda mitad (08/09): EL MODO INTELIGENTE OPERANDO CON UNA CAMARA AVERIADA
      // SE DICE. Hasta hoy no se decia, y no por falta de dato: el equipo publica CAM:
      // desde e3a21ec y esta app tenia el MODO y el ESTADO DE CAMARA en dos sitios SIN
      // CRUZARSE NUNCA. Faltaba la frase, no la telemetria.
      //
      // ESTO NO ES UNA BARRERA Y NO SE DISFRAZA DE UNA: no cambia ni una luz. El firmware
      // ya degrada solo y bien -modo_inteligente.cpp lo lleva escrito y razonado: "CON LAS
      // CAMARAS MUERTAS ESTE MODO SE COMPORTA EXACTAMENTE COMO EL AUTOMATICO ... degrada al
      // comportamiento conocido, no a uno raro"-. Lo que faltaba era que alguien se
      // enterase, que es otra cosa.
      //
      // Y SE DEJA EN LA APP A PROPOSITO, en vez de que el equipo se cambie solo de modo:
      // que la maquina decida operar en un modo que nadie pidio es justo lo que la barrera
      // de salidas prohibe, y ademas no compraria nada -el comportamiento ya es ese-.
      //
      // SON DOS FRASES Y NO UNA PORQUE LAS DOS AVERIAS FALLAN EN DIRECCIONES OPUESTAS:
      //   CIEGA  nunca dice "hay coches" -> la condicion de mantener no se cumple jamas y
      //          la fase acaba en el SUELO. El operario cree tener demanda y tiene un ciclo
      //          fijo: pierde la funcion por la que eligio este modo, sin sintoma.
      //   PEGADA dice "hay alguien" siempre -> MANTIENE mientras el otro lado no pida, hasta
      //          el TECHO (el doble del tiempo configurado). No es peligroso -nadie espera
      //          enfrente- pero el verde se alarga y nadie sabe por que.
      // Meterlas en una sola frase mandaria al tecnico a buscar el sintoma contrario.
      const CAM_EN_INTELIGENTE = {
        'CIEGA':  ' | MODO INTELIGENTE SIN DEMANDA: esta funcionando como el Automatico. Revise esa camara.',
        'PEGADA': ' | MODO INTELIGENTE ALARGANDO VERDES: mantiene hasta el techo aunque no pase nadie. Revise esa camara.'
      };

      const info = CAM_LEYENDA[state.cam];
      if (!info) {
        // LLEGO ALGO QUE NO PUEDE SER, y eso NO es '--'. '--' significa "todavia no lo
        // se" y aplastar aqui un valor imposible lo esconderia entre los huecos normales.
        // El literal en crudo va al renglon de abajo: sin el, el que mira no puede decir
        // si el equipo estreno una palabra o si la trama llego rota.
        decir('!', 'Valor no reconocido: "' + state.cam + '"', 'var(--red-lamp)');
        return;
      }
      // El aviso solo se anade cuando SE SABE el estado de la camara: si el valor no se
      // reconoce, o no vino, o el enlace esta caido, ya se salio por arriba. Afirmar ahi
      // que el modo esta degradado seria decidir sobre un dato que no se tiene.
      const aviso = (state.modo === 'INTELIGENTE' && CAM_EN_INTELIGENTE[state.cam]) || '';
      decir(info.texto, info.frase + aviso, info.color);
    }

    function renderLights() {
      if (!s1Red || !s1Amber || !s1Green || !s2Red || !s2Amber || !s2Green) return;

      [s1Red, s1Amber, s1Green, s2Red, s2Amber, s2Green].forEach(l => l.classList.remove('active'));

      // Cual de las dos columnas es la que habla. El reparto no se inventa aqui: es el
      // mismo que ya rotula el HTML -POSTE 1 = MAESTRO, POSTE 2 = ESCLAVO- y el mismo
      // que usa la cabecera al leer NODE:.
      const esEsclavo = state.node === 'ESCLAVO';
      // SPEC_4 9.9 (c): la UNICA funcion que escribe #phase-desc; con el equipo callado, "SIN ENLACE" (marcarSinEnlace()).
      const fase = (t) => { if (phaseDescEl) phaseDescEl.textContent = state.telemetriaViva ? t : 'SIN ENLACE - sin datos del equipo'; };
      const propias = esEsclavo ? [s2Red, s2Amber, s2Green] : [s1Red, s1Amber, s1Green];
      const ajenas = esEsclavo ? [s1Red, s1Amber, s1Green] : [s2Red, s2Amber, s2Green];
      const textoPropio = esEsclavo ? s2Text : s1Text;
      const textoAjeno = esEsclavo ? s1Text : s2Text;
      const rotuloAjeno = esEsclavo ? 'POSTE 1 · MAESTRO' : 'POSTE 2 · ESCLAVO';

      // EL OTRO EXTREMO, QUE DESDE N-149 A VECES SI SE PUEDE PINTAR.
      //
      // Hasta el 04/09 esto era `declararAjeno()` y no tenia mas remedio: el $STATUS
      // llevaba UN solo ESTADO -el del que lo manda- y el Maestro no publicaba el del
      // otro. Ahora publica ESC:, y la consola de operacion pinta LOS DOS POSTES.
      //
      // LAS TRES SALIDAS SON DISTINTAS Y NINGUNA SE COLAPSA CON OTRA, que es todo lo que
      // este bloque tiene que hacer bien (CLAUDE.md 3.quinquies): tres lamparas apagadas
      // a secas se leen como "ese poste esta apagado", y el ultimo color conocido
      // repintado se lee como el de ahora. Las dos serian afirmaciones sin medida.
      //
      // Devuelve la frase con la que la linea del centro nombra al otro poste, para que
      // el rotulo y la columna no se puedan desincronizar: los escribe la misma decision.
      function pintarAjeno() {
        // Las tres del otro poste se apagan al entrar y solo se vuelve a encender una si
        // hay dato de AHORA. Hoy el forEach de la cabecera de renderLights() ya las dejo
        // apagadas y esta linea no cambia nada; se escribe igual porque la propiedad que
        // importa -"una trama sin ESC: no puede dejar encendida la lampara de la trama
        // anterior"- tiene que vivir donde se decide, y no depender de que el barrido de
        // arriba siga estando ahi el dia que alguien lo mueva.
        ajenas.forEach(l => { if (l) l.classList.remove('active'); });
        const decir = (txt, color) => {
          if (!textoAjeno) return;
          textoAjeno.textContent = txt;
          textoAjeno.style.color = color || 'var(--text-muted)';
        };

        // Contra el ESCLAVO no hay nada del POSTE 1 y no lo va a haber: esa punta no
        // emite ESC: -no tiene a quien preguntarle- y esta pantalla es la ventana de
        // DIAGNOSTICO, no la consola de operacion. Se conserva la frase de siempre.
        if (state.node !== 'MAESTRO') {
          decir('SIN DATOS · se ve desde el POSTE 1');   // D-80
          return rotuloAjeno + ' no informa';
        }

        // El campo NO VINO: firmware anterior a N-149. Se dice CUAL es la carencia, no
        // "sin datos" a secas: sin eso, un equipo viejo y una radio caida se leen igual
        // en pantalla y mandan al tecnico a sitios opuestos.
        //
        // Y "no vino" es SOLO null/undefined. Un campo ESC con el valor vacio SI vino -el
        // equipo publico el campo y no puso nada dentro- y cae abajo, en el camino del
        // literal desconocido, para que se vea. Meterlo aqui acusaria de firmware viejo a
        // un equipo que acaba de mandar el campo, y taparia el unico sintoma que tiene un
        // %s vacio en el snprintf del C++.
        //
        // (El nombre del campo va SIN comillas invertidas a proposito. app_01_comandos
        // censa lo que la app manda buscando una raiz en mayusculas entre comillas
        // invertidas seguida de dos puntos y de algo pegado detras -asi es como se ve una
        // plantilla de trama-, y NO distingue una plantilla de un comentario. Escrito con
        // ellas, este parrafo hacia FALLAR al pack acusando a la app de mandar un comando
        // que ninguna punta conoce. Medido las dos veces, y las dos veces por el mismo
        // motivo: primero con el nombre del campo y luego con la palabra que lo describe.)
        if (state.esc === null || state.esc === undefined) {
          decir('SIN DATO DEL POSTE 2 · este equipo no lo publica');
          return rotuloAjeno + ': el POSTE 1 no publica su estado (firmware anterior)';
        }

        // El Maestro contesta que NO LO SABE. Es un dato, y de los buenos: dice que la
        // radio entre postes no esta entregando.
        if (state.esc === '?') {
          decir('SIN DATO DEL POSTE 2 · el POSTE 1 no lo sabe', 'var(--amber-lamp)');
          return rotuloAjeno + ': el POSTE 1 dice que NO SABE como esta (enlace entre ' +
                 'postes caído o aún sin respuesta)';
        }

        const luz = ESC_LUZ[state.esc];
        if (!luz) {
          // Un literal que esta tabla no conoce se ENSENA en crudo, igual que hace
          // pintarBadgeModo() con un MODO nuevo. Callarlo dejaria la columna en blanco el
          // dia que el firmware estrene un valor, y una columna en blanco es
          // indistinguible de una radio caida.
          decir('POSTE 2 NO RECONOCIDO: "' + state.esc + '"');
          return rotuloAjeno + ': valor no reconocido "' + state.esc + '"';
        }

        ajenas[{ red: 0, amber: 1, green: 2 }[luz.lampara]].classList.add('active');
        decir(luz.texto, luz.color);
        return null;   // 06/10: su columna ya lo dice; repetirlo partia la frase en ~9 renglones
      }

      pintarBadgeModo();
      // N-153: la fila de la pluma se pinta AQUI ARRIBA y no al final, para que salga por
      // los tres caminos de esta funcion -sin punta, ESTADO no reconocido y el normal-.
      // Solo lee state, asi que no depende de nada que se calcule mas abajo.
      pintarPluma();

      if (!state.node) {
        // Ni siquiera se sabe QUE punta hay al otro lado, asi que no hay columna que
        // pintar. Ocurre entre la conexion y el primer $STATUS.
        [s1Text, s2Text].forEach(t => {
          if (!t) return;
          t.textContent = 'SIN DATOS';
          t.style.color = 'var(--text-muted)';
        });
        if (phaseDescEl) phaseDescEl.textContent = state.connected ? 'EQUIPO SIN IDENTIFICAR - esperando su primer dato'
          : 'SIN ENLACE - sin datos del equipo';   // D-80: entre la conexion y el primer $STATUS
        return;
      }

      const info = ESTADOS[state.estadoLuces];
      if (!info) {
        // Llego un ESTADO que no esta en el enum del firmware. No se adivina: se ensena
        // el literal. Es lo que hace visible el desajuste en vez de dejar la pantalla
        // congelada, que es justo como este defecto sobrevivio.
        // El otro poste SE SIGUE PINTANDO: ESC: es un campo aparte y que el Maestro no
        // sepa nombrar SU luz no dice nada de lo que sabe de la otra punta. Tirar los dos
        // datos por uno malo seria perder el unico que quedaba bueno.
        const frAjeno = pintarAjeno();
        if (textoPropio) {
          textoPropio.textContent = 'ESTADO NO RECONOCIDO';
          textoPropio.style.color = 'var(--text-muted)';
        }
        fase((state.estadoLuces ? 'ESTADO no reconocido: "' + state.estadoLuces + '"'
                                : 'SIN ESTADO en la última trama') + ' · ' + frAjeno);
        return;
      }

      (info.lamparas || [info.lampara]).forEach(l => {
        propias[{ red: 0, amber: 1, green: 2 }[l]].classList.add('active');
      });
      if (textoPropio) {
        textoPropio.textContent = info.texto;
        textoPropio.style.color = info.color;
      }
      const frAjeno = pintarAjeno();

      // La linea del centro deja de anunciar una FASE DEL CRUCE -"FASE: SENTIDO 1"-,
      // que es una afirmacion sobre los dos postes, y pasa a decir DE QUIEN es el dato
      // que se esta viendo y que dice. Es la misma diferencia de arriba, escrita donde
      // el operario la lee. Desde N-149 la segunda mitad ya no es siempre "no informa":
      // la escribe pintarAjeno(), que es quien decidio que se pinto en esa columna.
      // info.frase NO se lee directo: FALLO COM tiene dos causas y solo MODO -de esta
      // MISMA trama- distingue cual es (_fraseDeEstadoLuces(), 28/09).
      fase((esEsclavo ? 'ESCLAVO' : 'MAESTRO') + ': ' + _fraseDeEstadoLuces(state.estadoLuces) + (frAjeno ? ' · ' + frAjeno : ''));
    }

    return { renderLights, pintarPluma, pintarCamaras };
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = Luces;
}
