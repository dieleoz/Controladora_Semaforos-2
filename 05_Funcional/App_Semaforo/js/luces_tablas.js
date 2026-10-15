// ===== js/luces_tablas.js =====
// Las tablas de la pintura de luces (ESTADOS, ESC_LUZ, MODOS, PLUMA_LEYENDA, CAM_LEYENDA), movidas
// literal de app.js el 15/10 (SPEC_4 9.12 tanda 2, D-98): mismo texto, dos espacios menos de sangria.
// Las lee js/luces.js, que pinta, y app.js (updateCountdownRing, el resumen del cruce) por su nombre.
// Cero nombres del cierre de app.js; app_04_valores_de_status las lee AQUI por ruta.

// $STATUS TRAE EL ESTADO DE UN SOLO POSTE, Y ESTA PANTALLA TIENE DOS.
//
// El vocabulario que este bloque leia -V1_R2, Y1_R2, R1_R2, ALL_RED, R1_V2, R1_Y2,
// AMBAR_FAIL- describe LOS DOS semaforos a la vez, y NO LO EMITE NINGUN FIRMWARE:
// sale del puente de PC (servidor_puente_simulador.py:28), que es un simulador.
// Lo que manda el equipo es el enum de semaforo_nombreEstado() (semaforo.cpp de las
// dos puntas, identicos): "ROJO" "ROJO+AMAR" (D-53) "VERDE" "AMARILLO" "FALLO COM".
// La interseccion con el del simulador es VACIA; lo vigila app_04_valores_de_status.
//
// No se renombran los `case`: `V1_R2` afirma algo de DOS postes y la trama habla de
// UNO: el que dice NODE:. La app NO TIENE el estado del otro extremo -no llega por
// Bluetooth, y ninguna punta lo pone en $STATUS-. Deducirlo -"si este da verde, el
// otro estara en rojo"- seria inventar justo la cifra que decide si se cruza, y
// seria FALSO precisamente cuando mas se mira el telefono: en Modo Degradado cada
// punta cicla por su cuenta, un ambar de emergencia es local, y con la radio caida
// las dos puntas se van a S_FALLO por separado.
//
// Asi que se pinta el poste que habla, con su valor literal, y del otro se dice que
// no se sabe. Un tablero quieto que admite que no sabe es honesto; uno que pinta un
// cruce que no ha medido le miente a quien decide sobre el trafico mirandolo (CLAUDE.md 3.quinquies).
//
// Y el otro sentido, que se cobra al reves: "FALLO COM" NO ES "APAGADO" NI ES ROJO.
// Es ambar intermitente CON LA TALANQUERA ARRIBA -SFTY-6, la politica que eligio el
// cliente el 27/08-: por ahi SE PASA con precaucion. El anillo lo pintaba de rojo
// (ver mas abajo), que dice lo contrario de lo que hace el equipo.
const ESTADOS = {
  'ROJO':      { lampara: 'red',   texto: 'ROJO (ESPERA)',  color: 'var(--red-text)',   anillo: 'red',
                 frase: 'ROJO, este poste no da paso' },
  // D-53 (05/10): antes de cada verde, rojo y amarillo encendidos a la vez. Es la
  // UNICA fila con dos lamparas (`lamparas`); `lampara` sigue siendo la que manda en el
  // anillo y en lo que lee una sola luz: rojo, porque todavia no da paso.
  'ROJO+AMAR': { lampara: 'red', lamparas: ['red', 'amber'], texto: 'ROJO Y AMARILLO',
                 color: 'var(--amber-lamp)', anillo: 'amber',
                 frase: 'Rojo y amarillo: va a abrir el verde' },
  'VERDE':     { lampara: 'green', texto: 'VERDE (PASO)',   color: 'var(--green-lamp)', anillo: 'green',
                 frase: 'VERDE, este poste da paso' },
  'AMARILLO':  { lampara: 'amber', texto: 'AMARILLO',       color: 'var(--amber-lamp)', anillo: 'amber',
                 frase: 'AMARILLO, este poste está cerrando su paso' },
  // FALLO COM es UN S_FALLO para dos causas (28/09): el ambar PEDIDO (SET_MODO:AMBAR) y la radio perdida; las
  // distingue MODO en la misma trama y lo pinta _fraseDeEstadoLuces(). Este `frase` es el de la radio perdida.
  'FALLO COM': { lampara: 'amber', texto: 'ÁMBAR DESTELLO', color: 'var(--amber-lamp)', anillo: 'amber',
                 frase: 'ámbar y pluma arriba: pasar con precaución' },  // la falla: cabecera y cartel
  // D-110 (SPEC_0 5.10 (a), (g)): el destello de la noche, pluma arriba. ROJO_INT solo lo da el POSTE 2.
  'AMBAR_INT': { lampara: 'amber', texto: 'ÁMBAR INTERMITENTE', color: 'var(--amber-lamp)', anillo: 'amber',
                 frase: 'noche: ámbar intermitente, pasar con precaución' },
  'ROJO_INT':  { lampara: 'red', texto: 'ROJO INTERMITENTE', color: 'var(--red-text)', anillo: 'red',
                 frase: 'noche: rojo intermitente, detenerse y pasar con precaución' }
};

// =========================================================================
// EL VOCABULARIO DE ESC:, QUE NO ES EL DE ESTADO: Y POR ESO ES OTRA TABLA
// =========================================================================
// N-149 (05/09). Decision del responsable: "cuando me conecto al maestro no me
// aparecen los estados del semaforo del esclavo... yo necesito que maestro me traiga
// los datos del esclavo". Y sobre lo que la app ofrecia hasta hoy -un boton para
// conectarse al otro poste-: "hay un problema porque ahi esta haciendo la conexion
// por Bluetooth, pero tendrias que caminar 1000 metros hasta el otro lado".
//
// POR QUE NO SE REUSA `ESTADOS`, que es la pregunta que hay que contestar antes de
// escribir una segunda tabla en este repositorio: son DOS campos distintos con DOS
// vocabularios distintos. ESTADO: sale de semaforo_nombreEstado() y puede valer
// "AMARILLO" o "FALLO COM"; ESC: es un resumen que el Maestro compone de lo que sabe
// del otro poste y su tercer valor es "AMBAR". Meter las claves de uno en la tabla
// del otro haria FALLAR a app_04_valores_de_status con razon -diria que la app sabe
// pintar un ESTADO que ninguna punta emite-, y ese pack es el que caza que el tablero
// no hable el idioma de un simulador.
//
// Lo que NO esta en esta tabla es '?' y esta escrito aparte a proposito: '?' no es un
// color, es el Maestro declarando que no lo sabe. Darle una fila aqui lo convertiria
// en un estado mas y es justo lo contrario de lo que significa.
const ESC_LUZ = {
  'ROJO':  { lampara: 'red',   texto: 'ROJO (ESPERA)', color: 'var(--red-text)' },
  'VERDE': { lampara: 'green', texto: 'VERDE (PASO)',  color: 'var(--green-lamp)' },
  'AMBAR': { lampara: 'amber', texto: 'ÁMBAR',         color: 'var(--amber-lamp)' }
};

// =========================================================================
// LA PLUMA, QUE ES LO UNICO DEL EQUIPO QUE SE MUEVE (N-153, 05/09)
// =========================================================================
// MEDIDO ANTES DE ESCRIBIR NADA: "talanquera", "pluma" y "barrera" daban 28
// coincidencias en este fichero y NINGUNA era un estado. Todas explican lo que hace
// un boton -"pone ambar y ABRE la talanquera"-, o sea que un operario delante del
// cruce no podia saber si la barrera estaba arriba o abajo AHORA. Y no era que la app
// lo ignorara: el equipo no lo publicaba. Desde N-153 viaja en el campo PLUMA del
// $STATUS de las dos puntas.
//
// POR QUE ES EL DATO QUE MAS FALTA, y no un adorno mas del tablero: el conductor le
// hace mas caso a la barrera que a la lampara -por eso se pone-. Con D-13 va a haber
// ratos de LUZ ROJA CON LA PLUMA ARRIBA -una camara ve presencia debajo y la barrera
// no baja-, y eso HOY un operario lo lee como averia y llama. La pantalla tiene que
// DECIRLO, no insinuarlo, y por eso la frase esta escrita entera abajo.
//
// LAS CUATRO SALIDAS NO SE COLAPSAN, igual que con ESC: arriba, abajo, el campo que
// no viene -firmware anterior- y un literal que esta tabla no conoce, que se ensena en
// crudo. La quinta -sin enlace- va aparte porque no es un valor: es que lo que hay en
// pantalla ya no vale.
// MODO: los literales de obtenerNombreModo() (Maestro/src/bluetooth.cpp) y los del Esclavo (SUBORDINADO, DEGRADADO,
// RENDIDO). Un modo sin fila dejaba el badge ANTERIOR, vencido con aspecto de vigente (DEGRADADO era invisible).
// `porEstado`: el texto segun el ESTADO de la misma trama (D-112 MENU, SPEC_1 4; D-110 NOCHE); sin fila, `texto`.
const MODOS = {
  'AUTO':        { texto: '🟢 AUTOMÁTICO',                fondo: 'rgba(0,230,118,0.15)',   borde: 'var(--green-lamp)', color: 'var(--green-lamp)' },
  'MANUAL':      { texto: '✋ MODO MANUAL',                fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
  'AMBAR':       { texto: '🟡 ÁMBAR PRECAUCIÓN',          fondo: 'rgba(255,179,0,0.15)',   borde: 'var(--amber-lamp)', color: 'var(--amber-lamp)' },
  'MENU':        { texto: '☰ EN MENÚ · SIN CICLO', fondo: 'rgba(0,240,255,0.15)', borde: 'var(--cyan-neon)',
                   color: 'var(--cyan-neon)',
                   porEstado: { 'ROJO': '☰ MENÚ · ROJO FIJO EN LOS DOS POSTES', 'AMARILLO': '☰ MENÚ · PASANDO A ROJO',
                     'ROJO+AMAR': '☰ MENÚ · PASANDO A ROJO', 'FALLO COM': '☰ MENÚ · ÁMBAR INTERMITENTE, SIN RADIO',
                     '': '☰ MENÚ · ÁMBAR INTERMITENTE, SIN RADIO' } },
  'NOCHE':       { texto: '🌙 NOCHE · DESTELLO', fondo: 'rgba(124,77,255,0.18)', borde: '#9c7cff', color: '#b9a2ff',
                   porEstado: { 'ROJO': '🌙 NOCHE · TODO-ROJO', 'AMARILLO': '🌙 NOCHE · PASANDO A ROJO',
                     'AMBAR_INT': '🌙 NOCHE · ÁMBAR INTERMITENTE' } },
  'INTELIGENTE': { texto: '👁 INTELIGENTE · POR DEMANDA',  fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
  'ALCANCE':     { texto: '🛑 ALCANCE · ROJO FIJO',       fondo: 'rgba(255,30,68,0.2)',    borde: 'var(--red-lamp)',   color: 'var(--red-lamp)' },
  'DEGRADADO':   { texto: '⚠️ DEGRADADO', fondo: 'rgba(255,179,0,0.15)', borde: 'var(--amber-lamp)', color: 'var(--amber-lamp)' },
  'SUBORDINADO': { texto: '🔗 SUBORDINADO AL MAESTRO',    fondo: 'rgba(0,240,255,0.15)',   borde: 'var(--cyan-neon)',  color: 'var(--cyan-neon)' },
  // A-11 (05/09). EL DECIMOPRIMERO, Y ES DEL ESCLAVO. Esa punta dejo de publicar
  // MODO:SUBORDINADO fijo el dia que se pudo pedir su Modo Degradado desde aqui: ahora
  // dice SUBORDINADO, DEGRADADO o RENDIDO segun donde este.
  //
  // RENDIDO NO ES UN DEGRADADO MAS FLOJO, Y POR ESO NO COMPARTE TEXTO. Es el Degradado
  // que TERMINO SOLO al vencer el limite de 48 h sin que el Maestro sincronizara la
  // hora, y la luz quedo en ambar intermitente. Pintarlo como DEGRADADO diria que el
  // cruce sigue operando por reloj cuando ya no opera: el tecnico leeria un modo
  // vigente donde hay una autorizacion caducada, que es lo contrario de lo que tiene
  // que hacer -volver a sincronizar y volver a entrar-.
  'RENDIDO':     { texto: '⏳ RENDIDO · ÁMBAR', fondo: 'rgba(255,179,0,0.15)', borde: 'var(--amber-lamp)', color: 'var(--amber-lamp)' },
  // Este no es "la app no lo conoce": es el equipo diciendo que no sabe en que modo
  // esta -el `default` de su propio switch-. Se distingue del de abajo a proposito.
  'DESCONOCIDO': { texto: '❔ EL EQUIPO NO SABE SU MODO',  fondo: 'rgba(148,163,184,0.15)', borde: 'var(--text-muted)', color: 'var(--text-muted)' }
};

const PLUMA_LEYENDA = {
  'ARRIBA': '▲ PLUMA ARRIBA',
  'ABAJO':  '▼ PLUMA ABAJO'
};

// =========================================================================
// LAS CAMARAS, QUE SON LO QUE VA A DECIDIR SI LA PLUMA BAJA (D-13 fase 1, 05/09)
// =========================================================================
// El equipo publica UNA sola palabra en CAM: y es LA PEOR de sus dos camaras. El
// orden de gravedad vive en el enum de botones.cpp -OK < ? < CIEGA < PEGADA- y aqui
// solo se LEE: una segunda copia de ese orden en este fichero seria la que se queda
// vieja el dia que la de alla cambie, y D-13 la va a cambiar.
//
// '?' NO ES 'OK', Y ES LA MITAD QUE MAS IMPORTA DE ESTA TABLA. Es el estado con el
// que arranca el vigilante: por esos pines todavia no ha pasado nada, asi que NO se
// sabe si la camara ve. Pintarlo en verde -o dejarlo caer en la misma casilla que
// "sin datos"- seria decirle al operario que la deteccion esta comprobada cuando lo
// unico que consta es que nadie la ha ejercido. Por eso el firmware le da mas peso
// que a OK y por eso aqui tiene fila propia.
//
// Y '?' TAMPOCO ES '--', que es la otra confusion facil. '--' es la APP diciendo que
// no tiene dato -no hay enlace, o el campo no vino-; '?' es el EQUIPO diciendo que el
// dato que tiene es "todavia no lo se". Mandan a sitios distintos: uno a mirar la
// radio, el otro a esperar o a provocar una deteccion.
const CAM_LEYENDA = {
  'OK':     { texto: 'OK',           frase: 'las dos ven y ninguna esta pegada',
              color: 'var(--green-lamp)' },
  '?':      { texto: 'SIN COMPROBAR', frase: 'aun no ha pasado nada por esos pines: no consta que vean',
              color: 'var(--text-muted)' },
  'CIEGA':  { texto: 'CIEGA',        frase: 'lleva demasiado tiempo de paso abierto sin una sola deteccion: puede estar tapada o sin corriente',
              color: 'var(--amber-lamp)' },
  'PEGADA': { texto: 'PEGADA',       frase: 'contacto fijo: dice que hay alguien y no se suelta',
              color: 'var(--red-lamp)' }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { ESTADOS, ESC_LUZ, MODOS, PLUMA_LEYENDA, CAM_LEYENDA };
}
