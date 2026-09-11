// ===== js/avisos_equipo.js =====
// Lo que el equipo AVISA por su cuenta -$ALARM y $EVENT-, traducido a LO QUE HAY QUE
// HACER en el poste.
//
// ES EL MISMO MOLDE QUE ERR_MOTIVO / ACK_TEXTO DE app.js: una entrada por literal del
// firmware, con {tono, texto, toast}; el literal en crudo NO se sustituye -la rama que
// pinta pone los dos, crudo primero y traduccion detras-; y lo que la tabla no nombra se
// ensena en crudo, que es la mitad que impide que esto se convierta en un filtro.
//
// POR QUE VIVE AQUI Y NO DENTRO DE app.js, que es la pregunta que hay que contestar antes
// de abrir un fichero nuevo en esta app. No depende de nada de la pantalla -ni de `state`
// ni del DOM: recibe los campos de la trama y devuelve texto-, y asi las dos suites
// unitarias pueden cargar la MISMA tabla que corre en el telefono en vez de copiarla.
// Una tabla de textos que solo se pudiera probar reescribiendola en la prueba seria la
// "segunda copia escrita a mano" que este repositorio ya pago con el parser (N-62).
//
// D-26 (11/09) - LA HORA LA MANDA EL ESP32 DE CADA POSTE, Y HAY DOS AVERIAS DISTINTAS:
//
//   HORA_ESP32  el enlace ESP32<->STM32 de la MISMA placa (J17) no pasa la hora, o la
//               pasa estropeada. Se arregla con destornillador en ese gabinete.
//   FALLO_RF    se perdio la radio entre postes. Sin radio el Esclavo toma la hora de su
//               propio ESP32, asi que hay que ponersela desde el telefono en SU gabinete.
//
// D-21 (2): la alarma se ve cuando el telefono esta conectado A ESE NODO -cada ESP32
// solo sube lo de su propia controladora-. Por eso los textos dicen "este poste" y lo
// nombran con el NODE que trae la propia alarma, no con el que la app crea tener.
//
// NINGUN TEXTO LLEVA UNA CIFRA DEL FIRMWARE (CLAUDE.md 14): ni la cadencia de la
// siembra, ni la espera de la alarma, ni el margen del cruce. Esta app no puede
// recalcularlas desde el C++ y cualquier numero copiado aqui nace caducado. Se dice lo
// que no envejece: "sigue saliendo mientras dure", "mas de lo que el cruce aguanta".
//
// Y NINGUN TEXTO LLEVA LA PALABRA DE LA ORDEN ENTRE COMILLAS DE CABLE: el pack
// app_07 censa como emisor de trama todo literal de js/ que contenga el prefijo de
// orden, y un texto que lo citara se leeria como un segundo generador.

// El poste que AVISA, dicho detras de "este poste". Sale del NODE de la propia trama.
function _cual(data) {
  const n = data && data.NODE;
  if (n === 'MAESTRO') return ' (MAESTRO, poste 1)';
  if (n === 'ESCLAVO') return ' (ESCLAVO, poste 2)';
  // Sin NODE no se adivina: una alarma atribuida al poste equivocado manda a abrir el
  // gabinete que no es.
  return ' (la trama no dice cual)';
}

// Lo que pasa con la hora MIENTRAS NADIE LO ARREGLA, que no es lo mismo en las dos
// puntas: D-26 (3), con radio la del Esclavo la manda el Maestro.
function _mientrasTanto(data) {
  if (data && data.NODE === 'ESCLAVO') {
    return 'Mientras haya radio, la hora de este poste la sigue mandando el Maestro; ' +
           'si se pierde la radio, este poste se queda sin nadie que se la corrija.';
  }
  return 'Mientras tanto la controladora sigue con la hora que tenia, sin nadie que se ' +
         'la corrija desde su ESP32.';
}

const AvisosEquipo = {
  // ---------------------------------------------------------------------------------
  // $ALARM. Clave 'EVENTO|CAUSA'; y 'EVENTO' a secas SOLO cuando lo que hay que hacer no
  // depende de la causa (FALLO_RF: la causa es el silencio medido, con su cifra dentro).
  //
  // Las tres de HORA_ESP32 van por causa y SIN entrada por EVENTO a proposito: mandan a
  // sitios distintos -dos a un destornillador, una al boton de sincronizar-, y una causa
  // nueva del firmware tiene que salir en crudo, no con la instruccion de otra.
  // ---------------------------------------------------------------------------------
  ALARMA: {
    // No llega NADA por J17, ni el latido. Si el telefono lo esta leyendo, el ESP32 esta
    // vivo y el sentido controladora->ESP32 funciona: lo roto es el otro sentido.
    'HORA_ESP32|J17_MUDO': (data) => ({
      tono: 'red',
      texto: 'REVISE EL CIRCUITO ESP32-STM32 DE ESTE POSTE' + _cual(data) + ': es ' +
             'la misma placa, y la controladora no recibe NADA de su modulo ESP32 por el ' +
             'cable interno J17, ni la hora ni el latido. Mire el conector J17 y el hilo ' +
             'que va del ESP32 a la controladora. ' + _mientrasTanto(data) + ' Ponerle la ' +
             'hora desde el telefono NO lo arregla: entra en el reloj del ESP32 y no llega ' +
             'a la controladora, aunque el acuse diga OK.',
      toast: 'Hora: la controladora no oye a su ESP32 - revise el circuito ESP32-STM32 (J17)'
    }),
    // El ESP32 solo manda horas que su reloj ya dio por buenas; si la controladora la tira
    // por la forma, la estropeo el camino.
    'HORA_ESP32|RECHAZADA_FORMATO': (data) => ({
      tono: 'red',
      texto: 'REVISE EL CIRCUITO ESP32-STM32 DE ESTE POSTE' + _cual(data) + ': es ' +
             'la misma placa, y la hora que el modulo ESP32 le manda a la controladora por ' +
             'el cable interno J17 llega cortada o mal formada, asi que la controladora la ' +
             'tira. El reloj del ESP32 no es el problema -solo manda horas que ya ha ' +
             'comprobado-: lo que la estropea es el camino entre los dos. Mire el conector ' +
             'J17, su cableado y que no haya un hilo flojo. ' + _mientrasTanto(data) +
             ' Ponerle la hora desde el telefono NO lo arregla: llegaria por el mismo camino.',
      toast: 'Hora rechazada por la controladora - revise el circuito ESP32-STM32 (J17)'
    }),
    // El latido si llega y la hora no: el reloj del ESP32 no tiene hora fiable y no
    // siembra sin ella. Es lo unico de las tres que arregla el telefono.
    'HORA_ESP32|SIN_HORA_DEL_ESP32': (data) => ({
      tono: 'red',
      texto: 'PONGALE LA HORA DESDE EL TELEFONO AQUI, EN EL GABINETE DE ESTE POSTE' +
             _cual(data) + ': pestana Tecnico, boton Sincronizar. La ' +
             'controladora si oye a su modulo ESP32, pero el reloj del ESP32 no tiene una ' +
             'hora fiable que darle, y sin ella no le manda ninguna. ' + _mientrasTanto(data) +
             ' Si al sincronizar el equipo contesta con un rechazo del reloj, ese motivo dice ' +
             'que tocar (pila o modulo). Si la hora queda puesta y este aviso vuelve a salir, ' +
             'el ESP32 de este poste no esta mandando la hora: revise que firmware lleva.',
      toast: 'Este poste no recibe hora de su ESP32 - pongasela desde el telefono (Tecnico > Sincronizar)'
    }),
    // D-21 (1): en Degradado la hora de esta punta llevo demasiado sin sembrarse y dejo de
    // poder decidir una luz: la punta se rinde a ambar. NO vuelve sola (D-21: el equipo no
    // decide solo si vuelve al modo), asi que el texto dice que hace falta alguien. Sin
    // cifras del plazo: la app no puede recalcularlo (CLAUDE.md 14).
    'HORA_ESP32|CADUCADA': (data) => ({
      tono: 'red',
      texto: 'ESTE POSTE' + _cual(data) + ' HA DEJADO EL MODO DEGRADADO Y ESTA EN AMBAR ' +
             'INTERMITENTE: su hora llevaba demasiado tiempo sin llegarle de su modulo ESP32 ' +
             'y ya no es fiable para decidir los verdes. Casi siempre es el circuito ' +
             'ESP32-STM32 de este poste (la misma placa, cable interno J17): reviselo, y ' +
             'ponga la hora desde el telefono en este gabinete (pestana Tecnico, boton ' +
             'Sincronizar). NO vuelve a dar paso solo: cuando la hora vuelva a llegar, hay ' +
             'que sacarlo del Modo Degradado y volver a entrar. Ojo: el otro poste puede ' +
             'seguir en Degradado dando verdes por su reloj.',
      toast: 'Degradado detenido: la hora de este poste caduco - en AMBAR hasta que alguien lo atienda'
    }),
    // D-26 (5): la otra averia. No es nueva -la emiten las dos puntas desde antes-, pero
    // D-26 le puso lo que hay que hacer, y hasta hoy salia en crudo. La causa NO se
    // traduce: lleva el silencio medido con su cifra, y esa cifra la da el equipo.
    'FALLO_RF': (data) => ({
      tono: 'red',
      texto: (data && data.NODE === 'MAESTRO'
               ? 'VAYA AL GABINETE DEL ESCLAVO (poste 2), CONECTESE A EL Y PONGALE LA HORA ' +
                 'DESDE EL TELEFONO: pestana Tecnico, boton Sincronizar. '
               : 'PONGALE LA HORA DESDE EL TELEFONO AQUI, EN EL GABINETE DE ESTE POSTE' +
                 _cual(data) + ': pestana Tecnico, boton Sincronizar. ') +
             'Se ha perdido la radio entre los dos postes: sin ella, el Esclavo ya no recibe ' +
             'la hora del Maestro y toma la de su propio ESP32, que es la que hay que dejar ' +
             'bien. Y revise la radio: el ultimo tramo que acompana a esta alarma dice como ' +
             'venia el enlace cuando se cayo.',
      toast: 'Radio perdida - ponga la hora al Esclavo desde el telefono, en su gabinete'
    })
  },

  // ---------------------------------------------------------------------------------
  // $EVENT. Clave 'ORIGEN|DETALLE'. El diario del equipo: SOLO en el cambio, no en cada
  // siembra, asi que cada una de estas lineas dice que algo acaba de cambiar.
  // ---------------------------------------------------------------------------------
  EVENTO: {
    // La primera siembra buena tras arrancar o tras una alarma de la hora.
    'ESP32|HORA_ESP32_SEMBRADA': (data) => ({
      tono: 'green',
      texto: 'La controladora de este poste' + _cual(data) + ' ha tomado la hora de su modulo ' +
             'ESP32. Si habia un aviso de la hora del ESP32 en este poste, ya esta resuelto.' +
             (data && data.NODE === 'ESCLAVO'
               ? ' En el Esclavo esto solo pasa sin radio: con radio, la hora la manda el Maestro.'
               : '')
    }),
    // Solo el Maestro (H6-i del 11/09): tomo la hora de su ESP32 pero no pudo encolar el
    // envio por radio al Esclavo. Hoy no deberia poder salir; si sale, el Esclavo se quedo
    // con la hora que tenia.
    'ESP32|HORA_ESP32_SEMBRADA_SIN_PROPAGAR': (data) => ({
      tono: 'red',
      texto: 'La controladora del Maestro' + _cual(data) + ' ha tomado la hora de su modulo ' +
             'ESP32, pero NO ha podido mandarsela por radio al Esclavo: el Esclavo sigue con ' +
             'la hora que tuviera. Compruebe la hora de los dos postes con Consultar reloj y, ' +
             'si no cuadran, reporte esta linea: es un defecto del firmware, no del cableado.'
    }),
    // Solo el Esclavo, y es lo normal con radio (D-26 (3)).
    'ESP32|HORA_ESP32_IGNORADA_MANDA_RADIO': () => ({
      tono: 'cyan',
      texto: 'Normal: en este poste la hora la manda el Maestro por radio, y la de su ' +
             'modulo ESP32 se deja a un lado a proposito. La del ESP32 solo manda aqui si se ' +
             'pierde la radio, asi que conviene que tambien este en hora: compruebela con ' +
             'Consultar reloj.'
    }),
    // D-26 (4): en Degradado, un salto de hora mayor que el margen pasa por rojo.
    'DEGRADADO|SALTO_DE_HORA_POR_ROJO': (data) => ({
      tono: 'red',
      texto: 'COMPRUEBE LA HORA DE LOS DOS POSTES (Consultar reloj en cada uno) Y, SI NO ' +
             'CUADRAN, PONGASELA DESDE EL TELEFONO EN CADA GABINETE. ' +
             'Este poste' + _cual(data) + ' esta en ' +
             'Modo Degradado, le ha llegado una hora que se aleja de la que tenia mas de lo ' +
             'que el cruce aguanta, y en vez de saltar de fase se ha puesto en ROJO a ' +
             'proposito. Vuelve a dar paso solo, por su reloj, al terminar el rojo de ' +
             'transicion. No es una averia del semaforo: es que las horas no cuadran.',
      toast: 'Degradado: salto de hora - este poste pasa por ROJO antes de volver a dar paso'
    })
  },

  // Devuelven {tono, texto, toast?} o null. null significa "esta app no lo sabe
  // traducir", y quien pinta lo ensena en crudo.
  traducirAlarma(data) {
    if (!data) return null;
    const ev = data.EVENTO || '';
    const entrada = this.ALARMA[ev + '|' + (data.CAUSA || '')] || this.ALARMA[ev];
    if (!entrada) return null;
    return typeof entrada === 'function' ? entrada(data) : entrada;
  },

  traducirEvento(data) {
    if (!data) return null;
    const entrada = this.EVENTO[(data.ORIGEN || '') + '|' + (data.DETALLE || '')];
    if (!entrada) return null;
    return typeof entrada === 'function' ? entrada(data) : entrada;
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = AvisosEquipo;
}
