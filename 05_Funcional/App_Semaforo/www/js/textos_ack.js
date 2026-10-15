// ===== js/textos_ack.js =====
// ACK_TEXTO: tabla pura de los $ACK, movida literal de app.js (SPEC_4 9.12, D-98); app.js la consulta igual.

const ACK_TEXTO = {
  // SPEC_4 9.5 (D-65, D-66): OK = la orden entra (la luz lo confirma); SIN_CAMBIO = ya estaba, no corrio nada.
  'SET_MODO:ALCANCE|OK': { tono: 'cyan',
    texto: 'Equipo: orden PRUEBA DE ALCANCE aceptada. Mire la luz: debe quedar en rojo fijo.',
    toast: 'Alcance aceptado: compruebe el rojo fijo' },
  'SET_MODO:ALCANCE|SIN_CAMBIO': { tono: 'amber',
    texto: 'Equipo: ya estaba en PRUEBA DE ALCANCE: no se movio nada. Para empezar de nuevo pase a MENU ' +
           'y vuelva a pedir PRUEBA DE ALCANCE.',
    toast: 'Ya estaba en PRUEBA DE ALCANCE: nada que hacer' },
  'SET_MODO:INTELIGENTE|OK': { tono: 'cyan',
    texto: 'Equipo: orden INTELIGENTE aceptada. El modo se ve en el rotulo del modo y en la luz, ' +
           'no en este aviso.',
    toast: 'Inteligente aceptado: compruebe el modo' },
  'SET_MODO:INTELIGENTE|SIN_CAMBIO': { tono: 'amber',
    texto: 'Equipo: ya estaba en INTELIGENTE: no se movio nada. Para reiniciar el ciclo pase a MENU ' +
           'y vuelva a pedir INTELIGENTE.',
    toast: 'Ya estaba en INTELIGENTE: nada que hacer' },
  // N-162: este $ACK dice que el Maestro ACEPTO la orden -su coordinador estaba en
  // reposo-, no que la otra punta vaya a abrir: si el Esclavo tiene un ambar puesto veta
  // el verde en silencio. Por eso no se promete el final, y no se cita ninguna cifra de
  // segundos: el despeje es configurable (coordinador_configurar) y la app no lo sabe.
  'CAMBIAR_TURNO|OK': {
    tono: 'green',
    texto: 'Equipo: CAMBIO DE TURNO ACEPTADO. El Maestro hace el todo-rojo de despeje ' +
           'configurado y despues da el paso. Mire las DOS cabezas: si una queda en ambar ' +
           'intermitente, el cambio NO se completo.',
    toast: 'Cambio de turno aceptado por el equipo'
  },
  'CANCELAR_AMBAR|RETIRADO': {
    tono: 'green',
    texto: 'Equipo: ambar de emergencia RETIRADO. No queda ningun ambar puesto en ' +
           'esa punta. Ojo: esta orden no mueve la luz, solo levanta el veto; la luz ' +
           'vuelve a moverse con la siguiente orden del Maestro.',
    toast: 'Ambar retirado por el equipo'
  },
  // LA QUE IMPORTA. El otro latch -el del mando del gabinete- NO se quita por radio:
  // los tres vetos de Esclavo/src/main.cpp miran las dos banderas, asi que con la del
  // mando puesta la luz sigue vetada. Un "ambar retirado" a secas manda al operario a
  // esperar un cambio de fase que no va a llegar hasta que alguien suba a hacer el
  // A.A.A, y mientras tanto el cruce sigue en ambar con la talanquera abierta.
  'CANCELAR_AMBAR|REENVIADO_AL_MAESTRO': {
    tono: 'red',
    texto: 'Equipo: en ESTA punta ya no queda ambar puesto desde la app -se quito en ' +
           'la pulsacion anterior-, asi que lo que se ha hecho ahora es VOLVER A ' +
           'PEDIRSELO AL MAESTRO, que es quien sostiene el ambar del cruce. NO esta ' +
           'garantizado que se vaya: si el ambar del Maestro lo pidio otra persona, esa ' +
           'punta ignora el aviso a proposito. Mire el cruce; si en unos segundos no se ' +
           'mueve, el ambar viene del Poste 1 y hay que resolverlo alli.',
    toast: 'Reenviado al Maestro: mire el cruce, no esta garantizado que se vaya'
  },
  // D-31 (12/09): este texto gana una frase, porque el literal ha cambiado de
  // significado sin cambiar de nombre. "OK" sigue queriendo decir "el ambar esta puesto
  // aqui", pero desde hoy la unidad ESPERA que el otro poste acuse el aviso, y ese
  // acuse todavia no ha llegado cuando sale este $ACK -el ambar no espera-. Si no
  // llegara, lo dice despues con un aviso propio. Sin esta frase, quien lee "OK" se va
  // creyendo que ya esta todo, que es el defecto que D-31 vino a cerrar.
  'AMBAR_EMERGENCIA|OK': {
    tono: 'red',
    texto: 'Equipo: AMBAR DE EMERGENCIA puesto. Intermitente en las dos vias y ' +
           'talanquera ABIERTA: no es un rojo, los dos sentidos pasan con precaucion. ' +
           'La unidad acaba de avisar por radio al otro poste y esta esperando que lo ' +
           'confirme: NO SE VAYA todavia. Si no lo consigue, sale un aviso rojo en esta ' +
           'misma pantalla diciendolo.',
    toast: 'Ambar puesto - esperando que el otro poste confirme el aviso'
  },
  // Ya estaba en ambar por SFTY-6, por el watchdog o por un B.B.B. Lo que esta orden
  // cambio NO es la luz: es el latch, que convierte un ambar que la siguiente orden
  // del Maestro se llevaria en uno vetado. Contestar "puesto" ocultaria que lo unico
  // nuevo es la proteccion, y con ella el tecnico decide distinto.
  'AMBAR_EMERGENCIA|YA_EN_AMBAR_LATCH_PUESTO': {
    tono: 'green',
    texto: 'Equipo: YA ESTABA en ambar por otro motivo. Lo que anade esta orden es la ' +
           'PROTECCION: a partir de ahora el ambar no se lo lleva la siguiente orden ' +
           'del Maestro. Para quitarlo hace falta RETIRAR AMBAR.',
    toast: 'Ya estaba en ambar - ahora ademas queda protegido'
  },
  // N-142 / §3.16-A (11/09). LAS DOS DE ABAJO SON LAS MISMAS DOS DE ARRIBA CON UN DATO
  // MAS, Y ES EL QUE DECIDE SI EL TECNICO SE PUEDE IR: el Poste 2 acaba de ponerse en
  // ambar y AVISA al Maestro por radio, pero esta punta ya habia declarado que se quedo
  // sin radio, asi que ese aviso casi seguro no llega. El Poste 1 no se entera y puede
  // seguir dando VERDE hacia el mismo carril. No es un rechazo -el ambar SI esta puesto
  // aqui- y por eso el tono no cambia; lo que cambia es lo que hay que hacer despues.
  'AMBAR_EMERGENCIA|OK_SIN_RADIO': {
    tono: 'red',
    texto: 'Equipo: AMBAR DE EMERGENCIA puesto EN ESTE POSTE, pero la unidad ha ' +
           'declarado que se quedo SIN RADIO: el otro poste NO se ha enterado y puede ' +
           'seguir dando verde hacia este carril. Cierre el paso alli o avise al ' +
           'Poste 1 antes de irse.',
    toast: 'Ambar puesto AQUI - sin radio: el otro poste no se ha enterado'
  },
  'AMBAR_EMERGENCIA|YA_EN_AMBAR_LATCH_PUESTO_SIN_RADIO': {
    tono: 'red',
    texto: 'Equipo: YA ESTABA en ambar y esta orden anade la PROTECCION (no se lo ' +
           'lleva la siguiente orden del Maestro). Pero la unidad ha declarado que se ' +
           'quedo SIN RADIO: el otro poste NO se ha enterado. Compruebe el Poste 1 ' +
           'antes de irse.',
    toast: 'Protegido, pero sin radio: el otro poste no se ha enterado'
  },
  // D-31 (12/09). LAS DOS DE ABAJO SON LAS UNICAS RESPUESTAS DE TODA ESTA TABLA QUE
  // PROMETEN ALGO DEL OTRO POSTE, Y LO PROMETEN PORQUE LA UNIDAD LO MIDIO: el Poste 1
  // ACUSO por radio un aviso anterior de este mismo ambar, o sea que hubo trama de
  // vuelta y la radio de salida de este poste funciona.
  //
  // LO QUE NO PROMETEN, y por eso el tono no es verde: que el Poste 1 SIGA en ambar. Su
  // ambar lo puede haber retirado alguien de aquel gabinete y esta punta no se entera.
  // Lo confirmado es la ENTREGA del aviso, no el estado del otro extremo.
  //
  // Salen a partir de la SEGUNDA pulsacion: la primera no puede decirlo todavia -el
  // acuse tarda un viaje de radio y el ambar no espera-, y si no llegara, lo que sale
  // es el aviso $ALARM del avisos_equipo.js, no un $ACK distinto.
  'AMBAR_EMERGENCIA|OK_POSTE1_AVISADO': {
    tono: 'red',
    texto: 'Equipo: AMBAR DE EMERGENCIA puesto, y el otro poste YA ESTABA AVISADO: ' +
           'confirmo por radio un aviso anterior de este mismo ambar, asi que la radio ' +
           'de salida de esta unidad funciona. No promete que el otro poste siga en ' +
           'ambar -eso lo puede haber quitado alguien alli-, solo que el aviso llego.',
    toast: 'Ambar puesto - el otro poste ya estaba avisado'
  },
  'AMBAR_EMERGENCIA|YA_EN_AMBAR_POSTE1_AVISADO': {
    tono: 'red',
    texto: 'Equipo: YA ESTABA en ambar con la proteccion puesta, y el otro poste YA ' +
           'ESTABA AVISADO: confirmo por radio el aviso de este mismo ambar. Esta ' +
           'pulsacion no ha cambiado nada, y eso es lo correcto. Si lo que quiere es ' +
           'quitarlo, use RETIRAR AMBAR.',
    toast: 'Ya estaba en ambar y el otro poste ya estaba avisado'
  },
  // El ambar NO esta puesto todavia: sale del Degradado por todo-rojo, de duracion el despeje en uso
  // (SPEC_2 7.quater (c)): sin cifra. Decir OK seria dar por hecho un cambio que aun no ha ocurrido.
  'AMBAR_EMERGENCIA|SALIENDO_TODO_ROJO': {
    tono: 'red',
    texto: 'Equipo: orden ACEPTADA pero el ambar TODAVIA NO ESTA PUESTO. La unidad ' +
           'esta saliendo del Modo Degradado por TODO ROJO, que dura el despeje en uso. ' +
           'El ambar entra al terminar esa salida: no se vaya sin verlo.',
    toast: 'Aceptada - el ambar entra al acabar el todo-rojo'
  },
  'AMBAR_EMERGENCIA|SALIDA_YA_EN_CURSO': {
    tono: 'red',
    texto: 'Equipo: orden ACEPTADA pero el ambar TODAVIA NO ESTA PUESTO. Ya habia una ' +
           'salida en curso (rendicion por el limite de 48 h) que termina en ambar; ' +
           'esta orden no la arranca, lo que anade es que ese ambar quede protegido.',
    toast: 'Aceptada - ya habia una salida en curso que acaba en ambar'
  },
  // N-146 (05/09, cinta de banco): AMBAR seis veces, OK las seis y el cruce quieto. OK = no
  // estaba en ambar y entra ahora; REARMADO = ya estaba en MODO_AMBAR con la luz en otra cosa
  // (un ROJO TOTAL mueve la luz y no el modo) y el ambar se vuelve a encender por un todo-rojo.
  'SET_MODO:AMBAR|OK': {
    tono: 'red',
    texto: 'Equipo: MODO AMBAR puesto AHORA. No estaba en ambar y acaba de entrar. ' +
           'Los DOS postes quedan en ambar intermitente: no es un rojo, los dos ' +
           'sentidos pasan con precaucion. No se vaya sin ver las dos puntas.',
    toast: 'Modo ambar: acaba de entrar'
  },
  'SET_MODO:AMBAR|REARMADO': {
    tono: 'red',
    texto: 'Equipo: YA ESTABA en modo ambar pero con la luz en otra cosa -un ROJO ' +
           'TOTAL lo deja asi-, y el ambar se ha VUELTO A ENCENDER con esta orden. ' +
           'La unidad pasa por un todo-rojo y vuelve a ordenar el ambar a las dos ' +
           'puntas: espere a verlo. Si pulso varias veces, ESTA es la que movio la luz.',
    toast: 'Ya estaba en modo ambar: el ambar se ha vuelto a encender'
  },
  // D-35: OK entra y queda en rojo hasta inicio; RENOVADO ya alternaba con ese ciclo.
  'SET_MODO:DEG_T|OK': {
    tono: 'red',
    texto: 'Equipo: TESTIGO ACEPTADO. Este poste queda en ROJO FIJO hasta la hora de ' +
           'inicio y despues alterna solo, sin radio. La cuenta atras esta en la tarjeta del testigo.',
    toast: 'Testigo aceptado: rojo fijo hasta la hora de inicio'
  },
  'SET_MODO:DEG_T|RENOVADO': {
    tono: 'green',
    texto: 'Equipo: TESTIGO RENOVADO. Este poste ya alternaba con ese mismo ciclo: ' +
           'la cuenta del testigo vuelve a empezar y SIGUE ALTERNANDO, sin volver a rojo ni esperar el inicio.',
    toast: 'Testigo renovado: la cuenta empieza de nuevo, sigue alternando'
  },
  // D-52 (SPEC_4 3.ter.ter): solo hay $ACK si quedo programada en ESTE poste; del otro no se sabe nada.
  'SET_MODO:DEG_FIN|PROGRAMADA': {
    tono: 'green',
    texto: 'Equipo: SALIDA PROGRAMADA en ESTE poste: a esa hora sale del Degradado por rojo. Cada poste sale ' +
           'a SU hora: si el otro no tiene la misma, sigue con verde. Programe la misma hora en los dos.',
    toast: 'Salida programada en este poste'
  },
  'SET_MODO:DEG_FIN|REPROGRAMADA': {
    tono: 'green',
    texto: 'Equipo: SALIDA REPROGRAMADA en ESTE poste: sustituye a la anterior. Del otro poste no se sabe ' +
           'nada: programe la misma hora en los dos.',
    toast: 'Salida reprogramada en este poste'
  },
  'SET_MODO:DEG_FIN|PROGRAMADA_SIN_RESPALDO': {
    tono: 'red',
    texto: 'Equipo: SALIDA PROGRAMADA en ESTE poste, pero SIN RESPALDO: un corte de luz la pierde y ' +
           'hay que volver a programarla despues de un corte.',
    toast: 'Salida programada SIN respaldo: un corte la pierde'
  },
  'SET_MODO:DEG_FIN|CANCELADA': {
    tono: 'cyan',
    texto: 'Equipo: salida CANCELADA en ESTE poste. Si el otro la tenia, sigue programada: ' +
           'cancelela alli.',
    toast: 'Salida cancelada en este poste'
  },
  'SET_MODO:MENU|OK': {
    tono: 'green',
    texto: 'Equipo: orden VOLVER AL MENU aceptada. Queda en el menu, sin ciclo: en rojo si el otro poste ' +
           'contesta por radio; sin radio, pasa a ambar intermitente.',
    toast: 'El equipo vuelve al menu'
  },
  // En Degradado no se salta al menu: se pide la salida, que es un todo-rojo de 30 s.
  'SET_MODO:MENU|OK_REANUDACION_CANCELADA': {
    tono: 'red',
    texto: 'Equipo: volvio al menu; el degradado que esperaba la hora NO se reanudara.',
    toast: 'Volvio al menu: el degradado NO se reanudara'
  },
  'SET_MODO:MENU|SALIENDO_TODO_ROJO': {
    tono: 'red',
    texto: 'Equipo: orden ACEPTADA pero TODAVIA NO ESTA EN EL MENU. Estaba en Modo ' +
           'Degradado y la salida pasa por un TODO ROJO de transicion; el menu llega ' +
           'cuando ese todo-rojo termine.',
    toast: 'Aceptada - sale del Degradado por todo-rojo antes de llegar al menu'
  },
  // SPEC_4 3.1 (D-45): fuera de Degradado, amarillo 3 s y rojo; en Degradado, su salida por todo-rojo.
  'FORZAR_ROJO|OK': {
    tono: 'red',
    texto: 'Equipo: ROJO TOTAL aceptado. Las dos vias pasan por 3 s de amarillo y quedan en ' +
           'ROJO FIJO. Compruebe las dos puntas antes de dejar pasar a nadie.',
    toast: 'Rojo total: amarillo 3 s y rojo fijo en las dos vias'
  },
  // D-51: en Degradado es rojo FIJO en este poste (no salida), y el otro sigue alternando por su reloj.
  'FORZAR_ROJO|ROJO_FIJO_EN_ESTE_POSTE': {
    tono: 'red',
    texto: 'Equipo: ROJO FIJO en ESTE poste. El OTRO POSTE SIGUE ALTERNANDO por su reloj: vaya a el. ' +
           'Se sale con la salida programada o con el boton Volver.',
    toast: 'Rojo fijo en este poste; el otro sigue alternando'
  },
  'FORZAR_ROJO|YA_EN_ROJO_FIJO': {
    tono: 'red',
    texto: 'Equipo: ya estaba en rojo fijo; nada cambia. El otro poste sigue por su reloj.',
    toast: 'Ya estaba en rojo fijo'
  },
  'FORZAR_ROJO|OK_REANUDACION_CANCELADA': {
    tono: 'red',
    texto: 'Equipo: ROJO FIJO en ESTE poste. El degradado que esperaba la hora NO se reanudara. ' +
           'El OTRO POSTE sigue por su reloj: vaya a el.',
    toast: 'Rojo fijo en este poste; el degradado NO se reanudara'
  },
  'FORZAR_ROJO|SALIDA_YA_EN_CURSO': {
    tono: 'red',
    texto: 'Equipo: el equipo YA ESTABA saliendo del Modo Degradado y ya esta en ROJO. ' +
           'Esta orden no cambia nada: al terminar el todo-rojo queda en el menu, sin ciclo.',
    toast: 'Ya estaba saliendo del Degradado, en rojo'
  },
  // 🔴 D-15 (05/09) - LOS DOS TEXTOS DE ABAJO DECIAN ALGO QUE YA NO PASA, Y ERA LO
  // PEOR QUE PODIAN DECIR: PROMETIAN LA OTRA PUNTA.
  //
  // Decian "hora puesta en el Maestro y propagada al Esclavo" y "entro en el MAESTRO
  // pero NO se propago al ESCLAVO". Los dos describian el $ACK del STM32, que
  // propagaba por radio con coordinador_sincronizarHora(). Ese comando ya no lo
  // atiende el STM32: quien contesta es el puente, que desde D-26 SIEMBRA a su propio
  // STM32 con la hora releida (ver el parrafo de abajo).
  //
  // Dejar el texto viejo habria sido la mentira con formato de exito una capa mas
  // arriba: el firmware deja de mentir y la app sigue diciendo lo mismo. El tecnico
  // leia "propagada", se iba, y el otro poste seguia con la hora que tuviera.
  //
  // El RESULT del puente significa OTRA COSA, y es lo que ahora se traduce:
  //   OK                        la hora entro en el DS3231 de ESTE poste Y la linea
  //                             llego entera al STM32 de ESTE poste.
  //   HORA_PUESTA_SIN_PROPAGAR  entro en el DS3231 de ESTE poste y la linea NO llego a
  //                             su STM32. "Propagar" aqui es J17 -el cable de dentro
  //                             del armario-, NO la otra punta del cruce.
  //
  // 🔴 D-26 (11/09) - Y EL TEXTO DE OK VOLVIO A QUEDARSE VIEJO, AHORA POR EL OTRO LADO.
  // Decia "esto NO toca el otro poste - no hay nada que los sincronice entre si", y desde
  // D-26 es FALSO para las CONTROLADORAS: el puente ya no reenvia los bytes del telefono,
  // siembra a su STM32 con la hora RELEIDA del DS3231, y el Maestro sembrado se la pasa
  // por radio al Esclavo (coordinador_sincronizarHora() dentro de su rama HORA_ESP32).
  // Lo que sigue siendo cierto es lo de los RELOJES CON PILA: el DS3231 del otro poste
  // no lo toca nadie, y sin radio es el que manda alli (D-26 (3)). El texto dice las dos
  // cosas y no promete lo que el acuse no sabe: "la mando entera" no es "la acepto" -si
  // la controladora la tira, lo dice ella con su propia alarma de la hora-.
  'SET_RTC|OK': {
    tono: 'green',
    texto: 'Equipo: hora puesta y RELEIDA del reloj de ESTE poste, y el modulo se la ha ' +
           'mandado entera a su controladora (si la controladora no la aceptara, lo ' +
           'diria con un aviso de la hora). En el MAESTRO, la controladora la pasa por ' +
           'radio al Esclavo; en el ESCLAVO, su controladora solo la usa sin radio, ' +
           'porque con radio manda la del Maestro. El reloj con pila del OTRO poste no ' +
           'se toca: pongale la hora tambien conectandose a el, que es el que manda alli ' +
           'si se pierde la radio.',
    toast: 'Hora puesta en ESTE poste - falta el reloj del otro'
  },
  // A-9. LA CONSULTA QUE NO CAMBIA NADA, Y POR ESO SE PUEDE HACER CON EL EQUIPO EN
  // MARCHA. Este texto NO repite los tres relojes ni el desfase: eso lo escribe
  // anotarLecturaDeReloj() en la linea de al lado, porque es un dato que cambia en
  // cada consulta y una tabla estatica no puede llevarlo. Lo que dice aqui es lo
  // unico que la RESPUESTA garantiza: que el modulo hablo y que la hora la acaba de
  // releer de su chip.
  // 1.51. EL TEXTO NO REPITE EL SELLO: ese viaja en el campo FW: de la propia trama y
  // cambia en cada equipo, asi que una tabla estatica no puede llevarlo -es lo mismo que
  // A-9 con los tres relojes-. Lo que dice aqui es lo unico que la RESPUESTA garantiza.
  'VERSION|OK': {
    tono: 'green',
    texto: 'Equipo: version contestada. La linea "Version cargada" es el commit ' +
           'del que salio el firmware que este equipo tiene CARGADO. Si termina en ' +
           '+SUCIO, ese binario se compilo sobre trabajo sin comitear: el commit NO lo ' +
           'describe entero y no sirve para acreditar una prueba de banco.',
    toast: 'Version contestada - vea Eventos'
  },
  'LEER_RTC|OK': {
    tono: 'green',
    texto: 'Puente: consulta de reloj contestada. La hora de abajo es la RELEIDA del ' +
           'chip en este instante, no una copia guardada. Esta orden no ha cambiado ' +
           'nada: ni el reloj, ni una luz, ni un modo.',
    toast: 'Reloj consultado - vea el desfase abajo'
  },
  // D-26 (5): lo que no cruza J17 es la averia "revisar el circuito" de la misma placa,
  // y el texto lo dice con esas palabras. Lo que no cruza ya no es "la orden" -desde
  // D-26 la orden del telefono se queda en el puente- sino la hora que el puente releyo.
  'SET_RTC|HORA_PUESTA_SIN_PROPAGAR': {
    tono: 'red',
    texto: 'Equipo: la hora entro en el reloj de ESTE poste, pero el modulo NO pudo ' +
           'mandarsela entera a su controladora por el cable interno (J17): la ' +
           'controladora sigue con la que tenia. El reloj esta bien; lo que falla es el ' +
           'camino de dentro del armario: REVISE EL CIRCUITO ESP32-STM32 de este poste ' +
           '(es la misma placa). El reloj del otro poste es aparte.',
    toast: 'Hora puesta en el reloj, pero no llego a la controladora: avise al tecnico (detalle en Eventos)'
  }
};

if (typeof module !== 'undefined' && module.exports) { module.exports = ACK_TEXTO; }
