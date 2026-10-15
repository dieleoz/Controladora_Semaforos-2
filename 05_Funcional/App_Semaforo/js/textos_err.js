// ===== js/textos_err.js =====
// ERR_TEXTO y ERR_MOTIVO: tablas puras de los $ERR, movidas literales de app.js (SPEC_4 9.12, D-98).

// Los rechazos ya se leen como rechazos lleve el texto que lleve, asi que aqui solo
// se traduce lo que el literal en crudo haria entender MAL. NO_HAY_AMBAR_VIGENTE
// parece una averia y no lo es: es el equipo diciendo que no habia nada que quitar,
// que es un dato util -si el cruce sigue en ambar, viene de otro sitio-.
const ERR_TEXTO = {
  'CANCELAR_AMBAR|NO_HAY_AMBAR_VIGENTE': {
    texto: 'Equipo: NO habia ningun ambar de emergencia puesto desde la app, asi que ' +
           'no se retiro nada. Si el cruce sigue en ambar viene de otro telefono o de un fallo de ' +
           'comunicacion (SFTY-6), y esta orden no quita ' +
           'ninguno de los dos.',
    toast: 'No habia ambar de la app que retirar'
  },

  // =======================================================================
  // A-9 - LOS SIETE "NO SE" DE LA CONSULTA DE RELOJ
  // =======================================================================
  // POR QUE ESTOS SI SE TRADUCEN, cuando la regla de arriba dice que un rechazo se
  // lee como rechazo lleve el texto que lleve.
  //
  // Porque aqui el rechazo NO es el final del asunto: es el dato. El operario ha
  // preguntado la hora, y la respuesta util no es "no se pudo" sino QUE ES LO QUE
  // FALLA -y sobre todo si el arreglo lo lleva en el bolsillo (repetir la orden), en
  // la caja (una pila) o en la furgoneta (un modulo)-. Los siete literales del puente
  // separan justo eso, y perder esa separacion en la pantalla es tirar el trabajo que
  // el firmware hizo para no dar un "no se pudo" generico.
  //
  // 🔴 Y LOS SIETE DICEN LO MISMO EN LA PRIMERA FRASE: LA HORA NO SE PINTA. Un
  // DS3231 sin pila entrega una fecha PERFECTAMENTE FORMADA y falsa, asi que la unica
  // salida honesta de todos ellos es la misma -no hay hora que ensenar-, y lo que
  // cambia detras es el arreglo. Es N-144: un cero con forma de hora es peor que un
  // hueco, porque el hueco no engana a nadie.
  // 1.51. NO ES UNA AVERIA DEL EQUIPO: es un binario que se compilo sin que git pudiera
  // decir de donde salia -o sin git-. Se dice asi para que nadie mande a revisar cobre.
  'VERSION|SIN_SELLAR': {
    texto: 'Equipo: este firmware NO SABE de que commit salio, y lo dice en vez de ' +
           'inventarlo. Pasa cuando se compilo fuera del repositorio -por ejemplo desde ' +
           'un .zip descomprimido sin git y sin su FW_HASH.txt-. El equipo funciona igual; lo ' +
           'que no se puede es atribuir una prueba de banco a un commit. Quien lo compilo ' +
           'tiene que decir el hash a mano.',
    toast: 'Firmware sin sellar: no se sabe de que commit salio'
  },
  'LEER_RTC|NUNCA_SE_PUSO_PONGA_LA_HORA': {
    texto: 'Puente: NO HAY HORA que ensenar en este poste, y no es una averia: es que ' +
           'nadie se la ha puesto todavia desde que el modulo arranco. Se arregla ' +
           'desde aqui mismo, con el boton de sincronizar de abajo.',
    toast: 'Este poste no tiene hora puesta todavia'
  },
  'LEER_RTC|OSCILADOR_PARADO_CAMBIE_PILA': {
    texto: 'Puente: NO HAY HORA FIABLE. El reloj tiene marcado que su oscilador se ' +
           'paro en algun momento, asi que lo que sus registros contengan no vale ' +
           'aunque tenga pinta de fecha. Es la pila del modulo del reloj: se cambia y ' +
           'se vuelve a poner la hora. Poner la hora sin cambiarla la pierde otra vez ' +
           'al primer corte.',
    toast: 'Reloj parado: cambie la pila del modulo'
  },
  'LEER_RTC|SIN_RELOJ_NO_RESPONDE': {
    texto: 'Puente: NO HAY HORA. El reloj no contesta por su bus, asi que no es que ' +
           'esté en hora o no: es que no está. Modulo desenchufado, mal cableado, o ' +
           'SDA y SCL cruzados. Esto se arregla con destornillador, no repitiendo la ' +
           'orden.',
    toast: 'El modulo de reloj no responde'
  },
  'LEER_RTC|ESCRITURA_A_MEDIAS_REPITA_SET_RTC': {
    texto: 'Puente: NO HAY HORA FIABLE. Una puesta en hora anterior se corto a mitad ' +
           'y el modulo se quedo con parte vieja y parte nueva: puede componer una ' +
           'fecha perfectamente valida y equivocada. La duda no se levanta sola - ' +
           'vuelva a sincronizar y compruebe que esta vez acusa OK.',
    toast: 'Puesta de hora a medias: vuelva a sincronizar'
  },
  'LEER_RTC|MODO_12H_PONGA_LA_HORA': {
    texto: 'Puente: NO HAY HORA FIABLE. El reloj esta guardando la hora en formato de ' +
           '12 horas, y leida asi puede ir hasta DOCE HORAS equivocada con el modulo ' +
           'perfectamente sano. No se le toca el ajuste por nuestra cuenta -seria ' +
           'cambiarle la hora a un equipo que esta en la calle-: sincronice desde ' +
           'abajo y queda corregido de paso.',
    toast: 'Reloj en formato 12 h: sincronice para corregirlo'
  },
  'LEER_RTC|REGISTROS_INCOHERENTES': {
    texto: 'Puente: NO HAY HORA. Lo que hay dentro del reloj no compone una fecha ' +
           '(un mes 19, un dia 0). SON DOS AVERIAS POSIBLES Y EL EQUIPO NO PUEDE ' +
           'DISTINGUIRLAS: una puesta en hora cortada -se arregla sincronizando- o un ' +
           'modulo/bus que devuelve basura -se arregla cambiando el modulo-. Pruebe lo ' +
           'primero; si vuelve a salir esto, es lo segundo.',
    toast: 'El reloj devuelve una fecha imposible'
  },
  // Este NO es una averia del reloj: es una contradiccion del propio firmware del
  // puente -su barrera dice que hay hora y su lectura dice que no-. Se nombra distinto
  // a proposito, porque manda a un sitio distinto: al que escribio el firmware, no al
  // que subio al poste.
  'LEER_RTC|BARRERA_INCOHERENTE': {
    texto: 'Puente: el modulo se contradice a si mismo -dice tener hora fiable y no ' +
           'ha podido entregarla-. Esto NO es una averia del reloj ni se arregla en el ' +
           'poste: es un defecto del firmware del puente. Anote la hora y el poste y ' +
           'reportelo; no toque el reloj.',
    toast: 'Contradiccion en el puente: reportelo, no toque el reloj'
  },
  'LEER_RTC|MOTIVO_NO_CONTEMPLADO': {
    texto: 'Puente: no entrego la hora y dio un motivo que ni esta app ni su propio ' +
           'despachador saben nombrar. Es firmware mas nuevo que esta app, o un caso ' +
           'que nadie cableo. No de la hora de este poste por buena.',
    toast: 'Motivo de reloj desconocido: no de la hora por buena'
  }
};

// =========================================================================
// 4.sexies EL MOTIVO DEL RECHAZO SE TRADUCE A LO QUE HAY QUE HACER EN EL POSTE
// =========================================================================
// REPORTE DE BANCO DEL 04/09: "al pedir la hora de RTC de maestro a esclavo, cuando
// se envia me devuelve error". Eso es TODO lo que el tecnico podia saber, y no es
// culpa del firmware: el equipo si dice que falla. MEDIDO con grep sobre los tres
// despachadores -Maestro/src/bluetooth.cpp, Esclavo/src/bluetooth.cpp y
// ESP32_Expansion/src/despachador.cpp-: hay 23 literales de DESC distintos, y seis de
// ellos son SOLO del reloj. La app no leia ninguno para nada mas que imprimirlo en
// crudo detras de la palabra "Rechazo de Firmware".
//
// Y esos seis mandan a sitios OPUESTOS con el mismo aspecto en pantalla:
// SIN_RELOJ_NO_RESPONDE es un destornillador -el bus no contesta, y una de las causas
// es SDA y SCL cruzadas-; NO_QUEDO_PUESTA es una pila. Leidos como "error", los dos
// se contestan igual: repitiendo la orden, que en el primer caso no puede funcionar
// nunca.
//
// POR QUE ES UNA TABLA APARTE Y NO MAS FILAS DE ERR_TEXTO, que es la pregunta que hay
// que contestar antes de escribir una segunda tabla en este repositorio:
//
//   - ERR_TEXTO se direcciona por CMD|DESC y traduce lo que depende de la ORDEN.
//     NO_HAY_AMBAR_VIGENTE solo significa algo sabiendo que se pidio RETIRAR AMBAR.
//     Esa tabla la vigila app_10 contra los $ERR de las dos bluetooth.cpp.
//   - ERR_MOTIVO se direcciona por DESC a secas y traduce lo que NO depende de la
//     orden: una pila agotada se cambia igual la mandara quien la mandara. Es ademas
//     la unica forma de cubrir los seis del PUENTE, que viven en el despachador del
//     ESP32 -un fichero que app_10 no censa-.
//
// La regla para no acabar con la misma frase en las dos: un motivo esta en UNA de las
// dos, nunca en las dos. Si al escribir una entrada hay que nombrar la orden para que
// se entienda, va en ERR_TEXTO; si no, va aqui.
//
// Y LO QUE NO SE TRADUCE SE ENSENA EN CRUDO, que es la mitad que impide que esto se
// convierta en un filtro. El firmware puede ganar motivos manana sin que esta tabla
// se entere: un literal feo en pantalla es incomodo y util; tragarselo es volver al
// "error" de hoy, que es de donde venimos.
const ERR_MOTIVO = {
  // --- EL RELOJ. Los seis del puente (ESP32_Expansion/src/despachador.cpp) ---
  'SIN_RELOJ_NO_RESPONDE': {
    texto: 'El modulo de reloj NO CONTESTA en el bus. Esto no se arregla repitiendo la ' +
           'orden: o el modulo no esta puesto, o el cableado no llega, o SDA y SCL van ' +
           'cruzadas. Revise el conector antes de volver a mandar la hora.',
    toast: 'El reloj no contesta: revise el cableado SDA/SCL'
  },
  'ESCRITURA_FALLIDA': {
    texto: 'El reloj SI contesta, pero no acepto la escritura: la hora NO quedo puesta. ' +
           'Repita una vez. Si vuelve a salir lo mismo, el modulo esta fallando y hay ' +
           'que cambiarlo.',
    toast: 'El reloj no acepto la escritura: la hora no entro'
  },
  'NO_QUEDO_PUESTA': {
    texto: 'La hora se escribio, el modulo dijo que si, y al releerla NO coincidia. Eso ' +
           'apunta a la PILA del modulo de reloj: cambiela y repita. La hora que hay ' +
           'dentro ahora mismo no es la que usted mando.',
    toast: 'La hora no quedo puesta: cambie la pila del reloj'
  },
  'OSCILADOR_PARADO_CAMBIE_PILA': {
    texto: 'La hora entro en los registros pero el reloj NO ESTA CONTANDO: el oscilador ' +
           'sigue parado. Cambie la pila del modulo. Mientras no cuente, el equipo no ' +
           'tiene hora aunque el modulo muestre una.',
    toast: 'El reloj no cuenta: cambie la pila'
  },
  'MOTIVO_NO_CONTEMPLADO': {
    texto: 'El puente rechazo la orden por un motivo que su propio firmware no sabe ' +
           'nombrar. Apunte la hora y avise: es un caso nuevo, no una averia conocida, ' +
           'y no hay nada que tocar en el poste con este dato.',
    toast: 'Rechazo por un motivo que el firmware no sabe nombrar'
  },
  // D-26 (11/09): los dos rechazos nuevos del puente. Ninguno lo provoca un boton de
  // esta app -la app no manda la linea de hora ni una orden que el puente se quede sin
  // rama-, y por eso mismo hay que decir que significan si aparecen: el primero es
  // alguien tecleando a mano la orden reservada al modulo, el segundo un defecto del
  // firmware del puente. Sin traduccion saldrian como un "error" que manda a mirar el
  // cable.
  'LINEA_RESERVADA_AL_PUENTE': {
    texto: 'El modulo ESP32 de este poste descarto la orden ENTERA porque lleva dentro ' +
           'el nombre de la linea de hora que solo el propio modulo puede mandar a la ' +
           'controladora. No salio nada hacia el equipo. Esta app no manda esa orden: si ' +
           've esto, alguien la escribio a mano. Para poner la hora use el boton ' +
           'Sincronizar.',
    toast: 'Orden descartada: la linea de hora es del propio modulo'
  },
  'RECLAMADA_SIN_RAMA': {
    texto: 'El modulo ESP32 de este poste se quedo la orden pero no tiene como ' +
           'atenderla, asi que no salio nada hacia el equipo. No es una averia del ' +
           'poste ni del cable: es un defecto del firmware del modulo. Anote la orden y ' +
           'la hora y reportelo.',
    toast: 'El modulo se quedo la orden sin atenderla: reportelo'
  },
  'LINEA_DEMASIADO_LARGA': {
    texto: 'La orden llego al puente mas larga de lo que cabe y se descarto ENTERA: no ' +
           'salio nada hacia el equipo. Si se repite siempre con la misma orden, el ' +
           'dato que lleva dentro es demasiado largo.',
    toast: 'El puente descarto la orden: llego demasiado larga'
  },

  // --- EL RELOJ. Los tres del STM32 ---
  //
  // Los dos del Maestro nombran "la consulta del reloj", y N-114 midio que esa
  // consulta es MODO_HORA, a la que hoy NO SE PUEDE LLEGAR: se arma con dos
  // botonAceptar() y botonAceptar() devuelve false desde que PB14/PB15 dejaron de ser
  // pulsadores. Lo que SI pasa es que el firmware emite reportarBitsDelReloj() justo
  // detras de estos dos rechazos (bluetooth.cpp:608 y :643), y ese $EVENT lo pinta
  // esta misma app en el REGISTRO DE EVENTOS. Se manda alli, que es donde el dato
  // esta de verdad, en vez de repetir el nombre de una pantalla tapiada.
  'SIN_CRISTAL_VEA_CONSULTA_RELOJ': {
    texto: 'El Maestro no tiene reloj en marcha, asi que no acepta la hora. NO busque ' +
           'la consulta del reloj en el equipo: se hace desde esta app. El ' +
           'equipo acaba de publicar los bits del reloj en el REGISTRO DE EVENTOS de ' +
           'esta app, justo debajo de este aviso: lealos ahi.',
    toast: 'Sin reloj en marcha: los bits estan en Registro de Eventos'
  },
  'SIGUE_PARADO_VEA_CONSULTA_RELOJ': {
    texto: 'El reloj INTERNO del Maestro sigue parado. Es lo normal en esta placa y NO ' +
           'es una averia: ese reloj no lleva la hora. La hora la guarda el reloj con pila ' +
           'del ESP32 (DS3231): compruebela con "Consultar reloj". Los bits quedan en el ' +
           'REGISTRO DE EVENTOS por si se los piden; no hace falta cambiar nada.',
    toast: 'Reloj interno parado (normal): la hora la lleva el DS3231'
  },
  // 1.49 (a): el tercer caso del reinicio. El oscilador SI arranco -por eso no es el de
  // arriba- pero el contador no se movio en la ventana en que el equipo lo vigila, asi que
  // el equipo NO dice "ponga la hora". Este rechazo llega unos segundos DESPUES de pulsar,
  // no en el acto: el equipo primero publica que esta verificando y contesta cuando lo sabe.
  // Por eso no se escribe aqui cuantos segundos: los decide el firmware.
  'ARRANCA_Y_NO_CUENTA_VEA_CONSULTA_RELOJ': {
    texto: 'Se reinicio el reloj y el oscilador ARRANCA, pero el contador NO AVANZA: el ' +
           'equipo lo vigilo unos segundos y no conto. NO ponga la hora: no la podria ' +
           'guardar. La consulta del reloj se hace desde esta app, no en el equipo. El ' +
           'equipo acaba de publicar los bits del reloj en el REGISTRO DE ' +
           'EVENTOS de esta app, justo debajo de este aviso: con el oscilador en marcha y el ' +
           'contador quieto, son los que dicen donde mirar.',
    toast: 'El reloj arranca pero no cuenta: los bits estan en Registro de Eventos'
  },
  'SIN_CRISTAL': {
    texto: 'Esa unidad no tiene reloj en marcha y por eso no acepta la hora. Ponga la ' +
           'hora en el MAESTRO: desde alli se propaga. Si el Maestro tampoco la ' +
           'acepta, el problema es de reloj y no de la app.',
    toast: 'Esa unidad no tiene reloj en marcha'
  },

  // --- LOS QUE NO SON DEL RELOJ ---
  'FORMATO_INVALIDO': {
    texto: 'El equipo no pudo leer los datos de la orden: llegaron incompletos o mal ' +
           'formados, y NO se ejecuto nada. Vuelva a rellenar el campo y repita. Si ' +
           'pasa siempre con la misma orden, avise: el aviso es del equipo, no del enlace.',
    toast: 'El equipo no pudo leer los datos de la orden'
  },
  'RANGO': {
    texto: 'Un valor de la orden se sale del rango que el equipo acepta y por eso la ' +
           'rechazo entera. Los limites los ensena el propio formulario: corrija el ' +
           'valor que este fuera y repita.',
    toast: 'Un valor va fuera del rango que acepta el equipo'
  },
  'EN_TRANSICION_REINTENTE': {
    texto: 'El cruce esta en mitad de un cambio de fase y no admite otra orden ahora ' +
           'mismo. NO es un fallo: espere a que termine el cambio y repita. Si insiste ' +
           'durante la transicion seguira saliendo lo mismo.',
    toast: 'El cruce esta cambiando de fase: repita al terminar'
  },
  'REPITA_EN_UNOS_SEGUNDOS': {
    texto: 'El equipo esta ocupado con lo anterior y no puede atender esta orden todavia. ' +
           'Espere unos segundos y repita. No hace falta tocar nada.',
    toast: 'El equipo esta ocupado: repita en unos segundos'
  },
  'SOLO_EN_MODO_INTELIGENTE': {
    texto: 'Esa orden solo vale con el equipo en Modo Inteligente, y ahora esta en otro ' +
           'modo. Ponga el equipo en Inteligente primero, o use la orden que corresponda ' +
           'al modo en que esta.',
    toast: 'Solo vale en Modo Inteligente'
  },
  'PIN_INVALIDO': {
    texto: 'El equipo RECHAZO la clave: la orden no se ejecuto. El PIN que manda la app ' +
           'sale del selector del modal de enlace; si al equipo le cambiaron el suyo, ' +
           'hay que cambiarlo alli.',
    toast: 'El equipo rechazo la clave'
  },
  'COMANDO_NO_SOPORTADO': {
    texto: 'El equipo NO CONOCE esa orden. Suele significar que la app o el manual son ' +
           'anteriores al firmware que hay cargado, o al reves. Anote la orden y la ' +
           'version del equipo antes de seguir probando.',
    toast: 'El equipo no conoce esa orden'
  },
  'COMANDO_NO_SOPORTADO_EN_ESCLAVO': {
    texto: 'Esa orden no la atiende el ESCLAVO. La mayoria de los mandos viven en el ' +
           'MAESTRO: enlace con el otro poste y repitala alli.',
    toast: 'Esa orden no la atiende el Esclavo: use el Maestro'
  },
  'NO_EN_SERVICIO_USE_EL_MAESTRO': {
    texto: 'El ESCLAVO rechaza el test de focos a proposito: la secuencia enciende VERDE ' +
           'sin mirar nada, y ese verde saldria mientras el Maestro puede estar dando ' +
           'paso al otro sentido. Hagalo desde el Maestro.',
    toast: 'El test de focos se hace desde el Maestro'
  },
  'RENOMBRADO_USE_AMBAR_EMERGENCIA': {
    texto: 'En el ESCLAVO la parada de emergencia ya no se llama ROJO TOTAL: alli la ' +
           'maniobra es AMBAR INTERMITENTE con la talanquera ABIERTA, y el mando que la ' +
           'pide es AMBAR EMERGENCIA. Use ese, que esta en esta misma pestana de mandos.',
    toast: 'En el Esclavo use AMBAR EMERGENCIA, no ROJO TOTAL'
  },
  'SALIDA_A_ROJO_EN_CURSO_REPITA': {
    texto: 'La unidad esta saliendo a todo-rojo ahora mismo y no admite la orden hasta ' +
           'que termine. Espere a que acabe la salida y repita: el ambar entrara ' +
           'entonces, no antes.',
    toast: 'Hay una salida a todo-rojo en curso: repita al acabar'
  },
  'YA_VUELVE_AL_MENU': {
    texto: 'El equipo YA estaba volviendo al menu, asi que esta orden no anade nada. No ' +
           'es un fallo: la vuelta atras ya estaba pedida y sigue su curso.',
    toast: 'El equipo ya volvia al menu'
  },

  // EL MOTIVO QUE VA A CAMBIAR, Y POR ESO ES UNA FUNCION Y NO UN OBJETO.
  //
  // Decision del responsable del 04/09: en vez de un rechazo seco, el equipo va a
  // decir CUANTOS SEGUNDOS faltan para poder parar el modo. Ese cambio es de firmware
  // y lo esta tocando otro; aqui queda el sitio hecho y NADA MAS.
  //
  // LO QUE NO SE HACE, Y ES LO IMPORTANTE: no se inventa el nombre del campo que
  // traera la cifra. Adivinarlo seria escribir una segunda copia de un contrato que
  // todavia no existe -y este repositorio ya pago tres veces esa clase de copia-, con
  // el agravante de que un nombre inventado no falla: simplemente no encuentra nada
  // NUNCA, y la cuenta atras quedaria muerta sin que ninguna prueba lo dijera. Hasta
  // que el campo exista, _segundosDeEspera() devuelve null a proposito y el texto sale
  // sin cifra, que es exactamente lo que la app sabe.
  // 🔴 AQUI NO VA UNA CUENTA ATRAS, Y NO ES QUE FALTE: ES QUE SERIA MENTIRA.
  //
  // Se comprobo el 04/09 antes de construirla. Los TRES sitios que emiten este motivo
  // rechazan por una condicion que EL TIEMPO NO CAMBIA:
  //
  //   SET_TIEMPOS ............ el ciclo automatico esta en marcha
  //   SET_MODO:ALCANCE ....... el equipo esta en Modo Degradado
  //   SET_MODO:INTELIGENTE ... el equipo esta en Modo Degradado
  //
  // En ninguno "esperar" sirve de nada: por muchos segundos que pasen el equipo sigue
  // en el mismo modo. Un contador diciendo "faltan 47 s" prometeria que aguantando se
  // arregla, y el tecnico se quedaria mirando el telefono hasta rendirse. El propio
  // literal del firmware lo dice bien -PARE_EL_MODO-, y la traduccion decia lo
  // contrario que el literal que traducia.
  //
  // Y NO ES EL MISMO CASO QUE EL AVISO DE VIA. Alli se pregunta porque el equipo no
  // sabe si quedan vehiculos y el operario si; aqui no hay nada que mirar: hay que
  // sacar el equipo del modo.
  // N-150: LA SEGUNDA MITAD DE LA FRASE FALTABA, Y ES LA QUE COSTO EL CRUCE PARADO.
  //
  // "y volver a entrar" estaba escrito y no dice COMO ni avisa de lo que pasa entre
  // medias. El operario del 05/09 hizo los dos primeros pasos -saco el equipo al
  // menu, mando los tiempos- y se quedo delante de un cruce en rojo esperando a que
  // arrancara solo. El paso que falta es suyo, y una instruccion que nombra tres
  // pasos y detalla dos deja al que la lee parado justo en el tercero.
  'EN_MARCHA_PARE_EL_MODO': () => ({
    texto: 'El equipo esta EN MARCHA y esa orden no se puede aplicar mientras lo este. ' +
           'ESPERAR NO SIRVE. Son TRES pasos y el tercero tambien es suyo: 1) VOLVER AL ' +
           'MENU -el cruce queda en ROJO en las dos puntas mientras tanto-; 2) mande los ' +
           'tiempos; 3) ARRANQUE EL CICLO con AUTOMATICO, porque el equipo NO vuelve a ' +
           'arrancar solo. Los tiempos no se tocan con el ciclo corriendo porque bajar ' +
           'uno a mitad de fase acortaria la fase EN CURSO, y una de esas fases es el ' +
           'todo-rojo que deja salir a los vehiculos del tramo.',
    toast: 'VOLVER AL MENU, mande los tiempos, y ARRANQUE con AUTOMATICO: son tres pasos'
  })
};

if (typeof module !== 'undefined' && module.exports) { module.exports = { ERR_TEXTO, ERR_MOTIVO }; }
