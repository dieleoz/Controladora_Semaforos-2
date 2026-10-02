# ===== banco/packs/maestro_09_test_leds.py =====
#
# EL TEST DE LAMPARAS NO TIENE UNA PUERTA PROPIA A LOS PINES (N-82).
#
# LO QUE ESTE PACK VIGILA Y LOS OTROS NO.
#
# barrera_01 comprueba que ningun fichero FUERA de semaforo.cpp escriba un pin de luz.
# Eso sigue siendo cierto y no se repite aqui. Lo que faltaba es la mitad de DENTRO:
# dentro de semaforo.cpp hay dos caminos hasta los pines y solo uno lleva la barrera.
#
#     aplicarSalidas()  -> enclavamiento SFTY-2 -> ultR/ultA/ultV -> escribirPines()
#     escribirPines()   -> los pines, sin mas
#
# El test de lamparas llamaba al de abajo. Un `grep escribirPines` daba doce llamadas
# y todas dentro del fichero permitido, asi que barrera_01 salia verde; el censo que
# hacia falta era otro: CUANTAS DE ESAS LLAMADAS PASAN ANTES POR EL ENCLAVAMIENTO.
#
# > La regla que queda: "todo pasa por una funcion" solo es una barrera si la barrera
# > esta EN esa funcion. Si vive un nivel por encima, basta llamar al nivel de abajo
# > para rodearla sin salirse del fichero, y ninguna guarda de rutas lo ve.
#
# LAS LLAMADAS DIRECTAS LEGITIMAS EXISTEN, Y POR ESO ESTO NO ES "CERO LLAMADAS".
#
# Los cuatro caminos de la senal del mando (SFTY-21) llaman a escribirPines() a
# proposito: INTERCEPTAN las escrituras en vez de rodearlas, para no dejar colgado al
# coordinador esperando un S_VERDE que no llegaria. Exigir cero llamadas seria una
# comprobacion que ningun firmware puede aprobar -CLAUDE.md §3-.
#
# Lo que si se puede exigir, y es la propiedad de verdad, es que NINGUNA de esas
# llamadas meta un VERDE CRUDO en los pines: la senal pide siempre rojo, ambar o todo
# apagado, y el volcado del final pide el ultV que el enclavamiento ya saneo. Un
# `escribirPines(false, false, true)` -que es literalmente el defecto de N-82- no tiene
# ningun sitio donde ser legitimo.
#
# LA TALANQUERA SE MIDE EVALUANDO LA CONDICION, NO BUSCANDO UN TEXTO.
#
# Comprobar que en escribirPines() aparece la cadena "testLedsActivo" seria medir la
# ortografia. Aqui se EXTRAE la condicion del ternario del pin y se EVALUA sobre su
# tabla de verdad, con la bandera del test descubierta del propio C++. Si la condicion
# deja de entenderse -un identificador que este pack no sabe leer- ABORTA, que es lo
# unico honesto: una expresion que no se sabe evaluar no se aprueba.
#
# POR QUE LA PLUMA ABAJO CON EL VERDE ENCENDIDO NO ES UNA CONTRADICCION.
#
# SFTY-28 dice que la pluma sigue al verde. La direccion peligrosa es una sola: pluma
# ARRIBA sin verde, porque el conductor le hace mas caso a la barrera que a la lampara.
# Al reves -verde con la pluma abajo- la barrera es MAS restrictiva que la luz, y el
# arnes del automatico lo dice con todas las letras al declarar su invariante. Un test
# de lamparas es exactamente ese caso: se ensena la lampara, no se concede el paso.

import re

# EJERCE SFTY-2: ningun verde llega a los pines sin pasar por el enclavamiento.
# EJERCE SFTY-28: la pluma sigue al verde de servicio y NO al verde de un test.

NOMBRE = "maestro_09_test_leds"
DESCRIPCION = "el test de lamparas entra por el enclavamiento y no levanta la talanquera"

RUTA = ("Maestro", "src", "semaforo.cpp")

# La UNICA funcion que puede llamar a escribirPines(). Es un TRINQUETE, no un
# absoluto: una funcion NUEVA aqui es un camino nuevo a los pines y tiene que
# discutirse, no colarse.
#
PUEDEN_ESCRIBIR_PINES = {
    "aplicarSalidas",          # el camino bueno: la barrera esta dentro
}

# El tercer argumento de escribirPines() que NO es un verde crudo. `false` es apagado;
# `ultV` es el verde que el enclavamiento ya decidio y guardo.
VERDE_ADMITIDO = ("false", "ultV")

_DEF = re.compile(
    r"^(?:static\s+)?(?:void|bool|uint8_t|unsigned\s+long|const\s+char\s*\*|"
    r"EstadoSemaforo)\s+(\w+)\s*\([^)]*\)\s*\{", re.M)


# El emparejador de llaves y el resolutor de la condicion de la pluma viven en
# banco/fuente.py: los necesitan tres packs y una copia por pack es el defecto que ese
# fichero existe para no cometer. Aqui solo queda el nombre corto.
from fuente import bloque as _bloque  # noqa: E402


def _funciones(codigo):
    """[(nombre, inicio, fin)] de cada definicion de funcion del fichero."""
    fuera = []
    for m in _DEF.finditer(codigo):
        i = codigo.index("{", m.end() - 1)
        tramo = _bloque(codigo, i)
        if tramo:
            fuera.append((m.group(1), tramo[0], tramo[1]))
    return fuera


def _quien_contiene(funciones, pos):
    for nombre, ini, fin in funciones:
        if ini <= pos < fin:
            return nombre
    return None


def _cuerpo(codigo, nombre):
    for n, ini, fin in _funciones(codigo):
        if n == nombre:
            return codigo[ini:fin]
    return None


# ---------------------------------------------------------------------------------
# LA CONDICION DE LA PLUMA, EVALUADA


_IDENT = re.compile(r"[A-Za-z_]\w*")


def _condicion_pluma(cuerpo):
    """(condicion, rama_si_cierto, rama_si_falso) del ternario de la talanquera."""
    plano = re.sub(r"\s+", " ", cuerpo)
    m = re.search(r"digitalWrite\(\s*MOTOR_TALANQUERA\s*,(.+?)\?\s*(\w+)\s*:\s*(\w+)\s*\)",
                  plano)
    if not m:
        return None
    return m.group(1).strip(), m.group(2), m.group(3)


def _evaluar_pluma(cond, bandera, verde, test, fallo, amarillo=False):
    """Evalua la condicion REAL del C++ con la tabla de verdad dada.

    No se reescribe la logica en Python -eso seria una segunda copia que alguien
    tendria que sincronizar, que es el defecto que este banco persigue-: se traduce
    la expresion y se evalua tal cual esta escrita en el fuente."""
    py = re.sub(r"estado\s*==\s*S_FALLO", "ES_FALLO", cond)
    py = re.sub(r"estado\s*==\s*S_AMARILLO", "ES_AMARILLO", py)   # D-45: el cierre
    py = py.replace("&&", " and ").replace("||", " or ").replace("!", " not ")
    return bool(eval(py, {"__builtins__": {}},  # noqa: S307
                     {"verde": verde, bandera: test, "ES_FALLO": fallo,
                      "ES_AMARILLO": amarillo}))


def correr(b, fw):
    b.titulo("N-82: el test de lamparas, por el enclavamiento y sin abrir la pluma")

    codigo = fw.codigo(*RUTA)
    funciones = _funciones(codigo)
    if len(funciones) < 10:
        raise fw.Abortado(
            "solo se reconocieron %d funciones en Maestro/src/semaforo.cpp. El lector "
            "de definiciones se quedo ciego, y un censo de llamadas sobre una lista "
            "casi vacia saldria en verde sin haber mirado nada" % len(funciones))

    # ---- 1. La bandera del test se DESCUBRE del C++, no se teclea aqui ----
    cuerpoInicio = _cuerpo(codigo, "semaforo_iniciarTestLeds")
    if cuerpoInicio is None:
        raise fw.Abortado(
            "no se encuentra semaforo_iniciarTestLeds() en Maestro/src/semaforo.cpp: "
            "sin ella este pack no sabe que bandera marca el test y compararia contra "
            "un nombre inventado")
    m = re.search(r"\b(\w+)\s*=\s*true\s*;", cuerpoInicio)
    if not m:
        raise fw.Abortado(
            "semaforo_iniciarTestLeds() ya no enciende ninguna bandera. O el test se "
            "arma de otra forma o el patron se quedo atras; en los dos casos lo que "
            "sigue no estaria midiendo el test")
    bandera = m.group(1)
    b.verificar(
        re.search(r"static\s+bool\s+%s\s*=" % re.escape(bandera), codigo) is not None,
        "la bandera del test se leyo del C++ y es `%s`, declarada static bool en el "
        "propio fichero" % bandera,
        "`%s` se enciende en semaforo_iniciarTestLeds() pero no se declara como "
        "`static bool` en semaforo.cpp. Este pack estaria siguiendo un simbolo que no "
        "es el que gobierna el test" % bandera)

    # ---- 2. La duracion de fase se relee del C++, SIN VALOR POR DEFECTO ----
    mFase = re.search(r"static\s+const\s+unsigned\s+long\s+(\w*FASE\w*)\s*=\s*(\d+)\s*;",
                      codigo)
    if not mFase:
        raise fw.Abortado(
            "no se pudo leer del C++ la duracion de fase del test (patron "
            "'static const unsigned long *FASE* = <n>'). Sin ese numero este pack "
            "mediria contra un valor escrito a mano, y el dia que el firmware lo "
            "cambiara seguiria dando PASS sobre el valor viejo")
    nombreFase, msFase = mFase.group(1), int(mFase.group(2))
    b.verificar(
        1000 <= msFase <= 5000,
        "cada fase dura %s = %d ms: bastante para que un tecnico confirme la lampara "
        "mirando hacia arriba, y no tanto como para que baje la vista antes del final"
        % (nombreFase, msFase),
        "%s vale %d ms. Por debajo de ~1 s no da tiempo a confirmar una lampara antes "
        "de que cambie la siguiente, y por encima de ~5 s el test de tres fases pasa "
        "de medio minuto y el tecnico deja de mirar" % (nombreFase, msFase))

    # ---- 3. NINGUN VERDE CRUDO EN LOS PINES ----
    #
    # La propiedad central. Se mira el TERCER argumento de cada escribirPines() que no
    # esta dentro de aplicarSalidas(): si alguno pide verde, ese verde llega al pin sin
    # que el enclavamiento lo haya visto.
    crudos = []
    llamadas = 0
    for mc in re.finditer(r"\bescribirPines\s*\(([^)]*)\)", codigo):
        quien = _quien_contiene(funciones, mc.start())
        args = [a.strip() for a in mc.group(1).split(",")]
        if len(args) != 3 or quien is None:
            continue
        llamadas += 1
        if quien == "aplicarSalidas":
            continue
        if args[2] not in VERDE_ADMITIDO:
            crudos.append("%s() pide verde=%s" % (quien, args[2]))

    # EL SUELO DEL CENSO, RE-DERIVADO EL 14/09 (D-30), Y CUAL ES EL BORDE.
    #
    # Era SEIS: la llamada de aplicarSalidas() mas las cinco de la senal del mando
    # -su hueco inicial, el ambar de rechazo, los dos destellos de actualizarSenal() y
    # el volcado de terminarSenal()-. Retirada la senal queda UNA, y el suelo se
    # RECALCULA desde lo que el fichero tiene hoy en vez de bajarse hasta que pase.
    #
    # POR QUE UNA ES EL NUMERO CORRECTO Y NO "poco": el fichero escribe los pines por un
    # solo sitio a proposito -es la barrera SFTY-2- y la comprobacion de mas abajo exige
    # que ese sitio sea aplicarSalidas() y ningun otro. Las dos juntas dicen lo mismo que
    # decia el seis y algo mas fuerte: no solo "he mirado caminos", sino "hay exactamente
    # un camino". CERO seria el caso malo de verdad -o el lector se rompio, o la barrera
    # se mudo de fichero y este pack estaria aprobando un semaforo.cpp que ya no escribe
    # una lampara (CLAUDE.md 5: la guarda no ve el contenido que se MUDA)-.
    b.verificar(
        llamadas >= 1,
        "censadas %d llamada(s) a escribirPines() en semaforo.cpp, clasificadas por la "
        "funcion que las contiene" % llamadas,
        "CERO llamadas a escribirPines() halladas en semaforo.cpp. O el lector dejo de "
        "entender la forma de la llamada, o el camino a los pines se mudo a otro "
        "fichero: en los dos casos este pack estaria aprobando sin haber mirado nada")

    b.verificar(
        not crudos,
        "ninguna llamada directa a escribirPines() mete un verde crudo: el unico verde "
        "que llega a los pines es el que el enclavamiento SFTY-2 ya saneo",
        "VERDE FUERA DEL ENCLAVAMIENTO: %s. Ese verde llega a la lampara sin que "
        "aplicarSalidas() lo haya visto, asi que SFTY-2 no lo ha enclavado y "
        "ultR/ultA/ultV no lo conocen -y de ese registro cuelga la reentrada de la pluma "
        "(D-33), que volcaria una foto vieja-. Y como la talanquera cuelga del mismo "
        "argumento, ademas abre la barrera" % ", ".join(crudos))

    # ---- 4. Quien llama directamente es SOLO la senal del mando ----
    directas = set()
    for mc in re.finditer(r"\bescribirPines\s*\(", codigo):
        quien = _quien_contiene(funciones, mc.start())
        if quien:
            directas.add(quien)
    intrusas = sorted(directas - PUEDEN_ESCRIBIR_PINES)
    b.verificar(
        not intrusas,
        "la unica funcion que llama a escribirPines() es aplicarSalidas(): todo verde "
        "que llega a una lampara ha pasado por el enclavamiento (%d camino/s censado/s)"
        % len(directas),
        "CAMINO NUEVO A LOS PINES: %s llama(n) a escribirPines() directamente. Desde "
        "D-30 no hay ninguna excepcion legitima: si no pasa por aplicarSalidas(), la "
        "barrera SFTY-2 no lo cubre y la pluma cuelga del mismo argumento" % ", ".join(intrusas))

    b.verificar(
        "semaforo_actualizar" not in directas,
        "semaforo_actualizar() ya no toca los pines por su cuenta: el test de lamparas "
        "sale por aplicarSalidas() como el resto del ciclo",
        "semaforo_actualizar() vuelve a llamar a escribirPines() directamente. Es el "
        "defecto de N-82 tal cual: el test de lamparas rodeando el enclavamiento sin "
        "salirse del fichero")

    # ---- 5. El test sigue ensenando las tres lamparas, y por turnos ----
    cuerpoAct = _cuerpo(codigo, "semaforo_actualizar")
    if cuerpoAct is None:
        raise fw.Abortado("no se encuentra semaforo_actualizar() en semaforo.cpp")
    i = cuerpoAct.find("if (%s)" % bandera)
    if i < 0:
        raise fw.Abortado(
            "no se encuentra el bloque `if (%s)` dentro de semaforo_actualizar(). El "
            "test se ejecuta en otro sitio o con otra forma, y este pack estaria "
            "midiendo un bloque que ya no es el del test" % bandera)
    tramo = _bloque(cuerpoAct, cuerpoAct.index("{", i))
    bloqueTest = cuerpoAct[tramo[0]:tramo[1]]

    fases = [tuple(a.strip() for a in g.split(","))
             for g in re.findall(r"\baplicarSalidas\s*\(([^)]*)\)", bloqueTest)]
    b.verificar(
        fases[:3] == [("true", "false", "false"),
                      ("false", "true", "false"),
                      ("false", "false", "true")],
        "el test pide las tres lamparas por turnos -rojo, ambar y verde- y las pide "
        "por aplicarSalidas(): el tecnico las ve, y cada una pasa por la barrera",
        "el test ya no ensena las tres lamparas en orden por aplicarSalidas(): pide "
        "%s. Un test de lamparas que se salta una lampara deja sin comprobar justo la "
        "que puede estar fundida" % (fases[:3] or "nada"))

    b.verificar(
        bloqueTest.count(nombreFase) >= 3,
        "las tres fases se cuentan sobre %s, no sobre numeros escritos a mano: no "
        "pueden desincronizarse entre ellas" % nombreFase,
        "el bloque del test usa %s menos de tres veces: alguna fase lleva su propio "
        "numero. Es N-71 otra vez -una relacion entre constantes que vive en la "
        "cabeza de quien la escribio y no en el codigo-" % nombreFase)

    # ---- 6. LA MAQUINA DE LUCES AVANZA EN TODOS LOS MODOS ----
    #
    # BLOQUE MUDADO LITERAL DESDE maestro_01_mando (D-30, 14/09), que se retira con el
    # mando. NO era una comprobacion del mando aunque viviera en su pack: lo que exige
    # es que main.cpp llame a semaforo_actualizar() SIN CONDICION dentro del loop().
    #
    # POR QUE SIGUE HACIENDO FALTA, con lo que cuelga de esa llamada HOY:
    #   - el parpadeo del ambar de S_FALLO, que vive dentro de semaforo_actualizar();
    #   - la transicion S_AMARILLO -> S_VERDE, tambien dentro;
    #   - y la REENTRADA DE LA PLUMA de D-33 -`if (plumaCierrePendiente) aplicarSalidas(
    #     ultR, ultA, ultV)`-, que es la unica forma de que la pluma baje cuando la
    #     camara deja de ver algo sin que cambie la luz. Sin esta llamada la pluma se
    #     queda ARRIBA hasta el proximo cambio de luz, que son minutos.
    #
    # Lo que este bloque exigia ANTES en su pack de origen era que el test de lamparas
    # esperase con una senal del mando en curso. Eso murio con la senal: no hay ya nada
    # que pueda ocupar las lamparas por encima de la logica, que es justo lo que hace
    # que esta llamada sea ahora el unico motor de las luces.
    #
    # maestro_10 tambien la mira, pero con reportar(), que NO CUENTA. Aqui cuenta.
    _main = fw.codigo("Maestro", "src", "main.cpp")
    _llamada_incondicional = bool(re.search(
        r"void loop\(\)[\s\S]{0,2000}?\n  semaforo_actualizar\(\);", _main))
    b.verificar(
        _llamada_incondicional,
        "main.cpp llama a semaforo_actualizar() SIN CONDICION en el loop(), asi que la "
        "maquina de luces avanza en todos los modos: el ambar de fallo parpadea, el "
        "amarillo pasa a verde y la pluma de D-33 puede volver a bajar",
        "main.cpp ya NO llama a semaforo_actualizar() sin condicion. Un modo que no "
        "llegue a esa llamada deja el ambar de S_FALLO CONGELADO -una lampara fija que "
        "no es ninguna senal del contrato- y, peor, deja la PLUMA ARRIBA hasta el "
        "proximo cambio de luz aunque la camara ya no vea nada (D-33)")

    # ---- 7. LA TALANQUERA, EVALUANDO LA CONDICION REAL ----
    cuerpoEscribir = _cuerpo(codigo, "escribirPines")
    if cuerpoEscribir is None:
        raise fw.Abortado("no se encuentra escribirPines() en semaforo.cpp")
    cond = _condicion_pluma(cuerpoEscribir)
    if cond is None:
        raise fw.Abortado(
            "no se pudo extraer el ternario de MOTOR_TALANQUERA de escribirPines(). "
            "Sin la condicion no hay nada que evaluar, y dar PASS aqui seria aprobar "
            "una barrera que no se ha mirado")
    expr, ramaCierto, ramaFalso = cond
    # N-153: la condicion puede venir envuelta en la asignacion de la bandera que el
    # $STATUS publica. Se desenvuelve SOLO si esa bandera es la que devuelve el getter,
    # comprobado sobre el fuente; en cualquier otro caso se deja como esta y se aborta.
    expr = fw.sin_asignacion(expr, codigo)
    # D-33: y desde el 14/09 puede ser un local, con la tabla de verdad una linea mas
    # arriba. Se la sigue hasta donde vive en vez de abortar; lo que NO se hace es dar
    # por buena una expresion que no se entiende.
    publicada = fw.bandera_publicada(codigo)
    d33 = fw.apertura_de_la_pluma(cuerpoEscribir, publicada, expr)
    if d33 is not None:
        expr = d33["tabla"]

    desconocidos = sorted(set(_IDENT.findall(expr)) -
                          {"verde", bandera, "estado", "S_FALLO", "S_AMARILLO"})
    if desconocidos:
        raise fw.Abortado(
            "la condicion de la pluma menciona %s, que este pack no sabe evaluar. Una "
            "expresion que no se entiende no se aprueba: se aborta y se viene a "
            "mirarla" % ", ".join(desconocidos))

    b.verificar(
        ramaCierto == "TALANQUERA_ABRIR" and ramaFalso == "TALANQUERA_CERRAR",
        "el ternario de la pluma abre con la condicion cierta y cierra con la falsa",
        "las ramas del ternario de la pluma son %r/%r. Si estuvieran cambiadas, toda "
        "la tabla de verdad de abajo se leeria del reves y este pack aprobaria una "
        "barrera invertida" % (ramaCierto, ramaFalso))

    def pluma(verde, test, fallo):
        return _evaluar_pluma(expr, bandera, verde, test, fallo)

    b.verificar(
        not pluma(True, True, False),
        "CON EL TEST EN CURSO Y EL VERDE ENCENDIDO, LA PLUMA SE QUEDA ABAJO: la lampara "
        "se ensena, el paso no se concede",
        "con el test en curso y el verde encendido la talanquera ABRE. Una prueba de "
        "lamparas levanta la barrera durante %d ms en un cruce en servicio, y el "
        "conductor le hace mas caso a la barrera que a la lampara" % msFase)

    b.verificar(
        pluma(True, False, False),
        "fuera del test, la pluma SIGUE al verde: SFTY-28 intacto para el paso de "
        "verdad",
        "con verde de servicio -sin test- la pluma se queda ABAJO. El arreglo de N-82 "
        "se paso de largo y dejo la barrera cerrada cuando el equipo si esta dando "
        "paso: eso es un corredor de obra sin salida")

    b.verificar(
        not pluma(False, False, False) and not pluma(False, True, False) and
        _evaluar_pluma(expr, bandera, False, False, False, True),
        "sin verde y sin fallo la pluma esta abajo, haya test o no; y con el amarillo de "
        "CIERRE sigue arriba (D-45, SPEC_8 1: el retardo cuenta desde el rojo)",
        "la pluma abre sin verde y sin S_FALLO. Es la direccion peligrosa: una barrera "
        "levantada invitando a pasar con la luz en rojo")

    b.verificar(
        pluma(False, False, True) and pluma(False, True, True),
        "en S_FALLO la pluma sigue subiendo -la politica de SFTY-6 que eligio el "
        "cliente el 27/08- y el test no se la lleva por delante",
        "la excepcion de S_FALLO se perdio: con el equipo sin enlace la barrera se "
        "quedaria ABAJO, cerrando la via por completo, que es la politica CONTRARIA a "
        "la decidida")

    # ---- 7.bis. LA RAMA QUE D-33 CREO, Y LA UNICA PREGUNTA QUE HAY QUE HACERLE ----
    #
    # El retardo y el veto son EXCEPCIONES A LA BAJADA, no permisos de subida. Escritas
    # mal serian lo segundo: bastaria que la rama de "pluma ya abajo" mirase la camara
    # para que una deteccion LEVANTARA la barrera con la luz en rojo. Ninguna de las
    # comprobaciones de arriba lo veria -la tabla de verdad de la apertura seguiria
    # intacta- y el arnes tampoco, porque ahi la pluma nunca parte de abajo con una
    # camara viendo algo. Por eso esta linea mira la FORMA: la rama de en medio cierra
    # y no consulta nada.
    if d33 is not None:
        b.verificar(
            d33["cierra"],
            "D-33: con la pluma YA ABAJO y la luz sin pedirla, el firmware la deja "
            "abajo en seco -sin mirar camara ni reloj-. El retardo y el veto solo "
            "pueden RETENER una pluma que ya estaba arriba, nunca ABRIRLA",
            "la rama de D-33 para la pluma ya cerrada no la deja cerrada. Si de ahi "
            "cuelga el veto, una deteccion de camara LEVANTA la barrera con la luz en "
            "rojo: la excepcion habria dejado de ser una excepcion a la bajada para "
            "ser un permiso de subida")

    # ---- 8. CONTROLES NEGATIVOS ----
    #
    # Sin esto, el dia que un patron dejara de casar este pack aprobaria un firmware
    # con el defecto dentro y nadie se enteraria. Cada uno ejerce el detector contra
    # el texto DEFECTUOSO de verdad, no contra uno inventado.
    b.control_negativo(
        _evaluar_pluma("(verde || ES_FALLO)".replace("ES_FALLO", "estado == S_FALLO"),
                       bandera, True, True, False),
        "la condicion ANTERIOR a N-82 -(verde || estado == S_FALLO)- sale evaluada "
        "como ABRIR con el test en curso: el evaluador distingue el arreglo del defecto")

    mutado = codigo.replace(
        "void semaforo_actualizar() {",
        "void semaforo_actualizar() { escribirPines(false, false, true);", 1)
    fmut = _funciones(mutado)
    reinyectado = [
        _quien_contiene(fmut, mc.start())
        for mc in re.finditer(r"\bescribirPines\s*\(\s*false\s*,\s*false\s*,\s*true\s*\)",
                              mutado)]
    b.control_negativo(
        "semaforo_actualizar" in reinyectado,
        "el verde crudo de N-82 reinyectado en semaforo_actualizar() se detecta y se "
        "atribuye a la funcion correcta")

    # D-33: y el detector de la rama de en medio, ejercido contra el texto defectuoso
    # de verdad -el que ABRE desde cerrada-, no contra uno inventado.
    _MALA = ("{ const bool L = (verde && !testLedsActivo) || estado == S_FALLO;"
             "  bool P;"
             "  if (L) { P = true; }"
             "  else if (!F) { P = camara_presenciaJ16(); }"
             "  else { P = false; }"
             "  digitalWrite(MOTOR_TALANQUERA, (F = P) ? TALANQUERA_ABRIR : X); }")
    _BUENA = _MALA.replace("P = camara_presenciaJ16();", "P = false;")
    b.control_negativo(
        fw.apertura_de_la_pluma(_MALA, "F", "P")["cierra"] is False
        and fw.apertura_de_la_pluma(_BUENA, "F", "P")["cierra"] is True,
        "una rama de D-33 que ABRE la pluma desde cerrada con la camara se detecta, y "
        "la que la deja cerrada pasa: el detector distingue la excepcion del permiso")

    b.control_negativo(
        fw.apertura_de_la_pluma(_BUENA, "F", "P")["tabla"]
        == "(verde && !testLedsActivo) || estado == S_FALLO",
        "y la tabla de verdad que se saca de la cadena de D-33 es la ENTERA, con sus "
        "tres terminos: si se quedara con un trozo, las cuatro filas de arriba se "
        "estarian evaluando sobre media condicion")

    b.control_negativo(
        _condicion_pluma("{ digitalWrite(MOTOR_TALANQUERA, (a && b) ? X : Y); }")
        == ("(a && b)", "X", "Y"),
        "el extractor del ternario devuelve la condicion entera y sus dos ramas, y no "
        "se queda con un trozo")

    # ---- 9. N-82.bis: SOLO FUERA DE SERVICIO, Y AL ACABAR DEVUELVE EL ESTADO ----
    #
    # Cinta del 28/09 (serie 4D2007): TEST_LEDS en AUTO con el Maestro en VERDE; al acabar,
    # luz ROJA con $STATUS en VERDE y la pluma abajo hasta cambiar de modo. Y medido en el
    # arnes del automatico: en AUTO con el Esclavo en verde, la fase verde del test encendia
    # el verde del Maestro 2 s -SFTY-2 solo enclava rojo/verde del MISMO poste-.
    #
    # EL BORDE, Y POR QUE ES ESE: se admite solo en los modos cuyo *_setup() llama a
    # coordinador_forzarMenu() -rojo fijo en las DOS puntas- y con la luz en S_ROJO. La
    # lista de modos NO se teclea: sale del switch de main.cpp y del cuerpo de cada setup.
    b.verificar(
        re.search(r"if\s*\(\s*!\s*testLedsAdmitido\s*\(\s*\)\s*\)", cuerpoInicio)
        is not None and "%s = true" % bandera in cuerpoInicio and
        cuerpoInicio.index("testLedsAdmitido") < cuerpoInicio.index("%s = true" % bandera),
        "semaforo_iniciarTestLeds() pregunta testLedsAdmitido() ANTES de armar `%s`"
        % bandera,
        "semaforo_iniciarTestLeds() arma el test sin preguntar testLedsAdmitido(): "
        "TEST_LEDS vuelve a correr en servicio")
    b.verificar(
        re.search(r"if\s*\(\s*%s\s*&&\s*!\s*testLedsAdmitido\s*\(\s*\)\s*\)\s*"
                  r"terminarTestLeds\s*\(\s*\)\s*;" % re.escape(bandera),
                  cuerpoAct) is not None
        and cuerpoAct.index("testLedsAdmitido") < i,
        "semaforo_actualizar() corta el test ANTES de su bloque si el equipo sale de "
        "fuera de servicio: un test pedido en MENU no sigue en el AUTO siguiente",
        "semaforo_actualizar() ya no corta el test al salir de fuera de servicio: un "
        "test pedido en MENU seguiria ensenando su verde dentro del ciclo")

    # El final: el literal de rojo fijo es el defecto de la cinta. Se exige que el
    # bloque del test acabe en terminarTestLeds() -y no en un cuarto aplicarSalidas()- y
    # que esta devuelva CADA estado por su propio setter.
    fin = _cuerpo(codigo, "terminarTestLeds")
    pares = dict(re.findall(r"case\s+(S_\w+)\s*:\s*(semaforo_\w+)\s*\(\s*\)\s*;", fin or ""))
    # D-45: el amarillo ya no es la apertura sino el cierre, y su setter (forzarRojo) no lo
    # repinta -un cierre en curso no se reinicia-: se devuelve el amarillo en la cara y el
    # cierre sigue con su reloj.
    esperado = {"S_ROJO": "semaforo_forzarRojo", "S_VERDE": "semaforo_forzarVerde",
                "S_FALLO": "semaforo_iniciarFallo"}
    amarilloOk = re.search(r"case\s+S_AMARILLO\s*:\s*iniciarTransicionARojo\s*\(\s*false\s*\)\s*;",
                           fin or "") is not None
    def _final_bueno(bloque):
        f = [g for g in re.findall(r"\baplicarSalidas\s*\(([^)]*)\)", bloque)]
        return "terminarTestLeds" in bloque and len(f) == 3

    # Control negativo: el final de antes de la cinta, reinyectado en el bloque REAL. Solo
    # se puede reinyectar si el bloque lleva el final nuevo; si no lo lleva, la
    # comprobacion de abajo ya sale en FALLA y el control no tendria sobre que operar.
    if "terminarTestLeds();" in bloqueTest:
        viejo = bloqueTest.replace(
            "terminarTestLeds();", "%s = false; aplicarSalidas(true, false, false);" % bandera)
        b.control_negativo(
            not _final_bueno(viejo),
            "el final de rojo fijo de antes del 28/09, reinyectado en el bloque real del "
            "test, se detecta")

    b.verificar(
        fin is not None and pares == esperado and amarilloOk and _final_bueno(bloqueTest),
        "al acabar el test la lampara vuelve a la luz de `estado` por su propio setter "
        "(%d estados): luz, estado y PLUMA: dicen lo mismo" % len(pares),
        "el final del test no devuelve la luz del estado (fases=%s, pares=%s). Es el "
        "defecto de la cinta del 28/09: rojo fijo con $STATUS en VERDE y la pluma abajo"
        % (fases, pares))

    admit = _cuerpo(codigo, "testLedsAdmitido")
    # Su AUSENCIA es un defecto del firmware, no una ceguera del pack: el test sin guarda
    # es justo lo que la cinta del 28/09 midio. Por eso FALLA y no ABORTADO.
    b.verificar(
        admit is not None,
        "semaforo.cpp tiene testLedsAdmitido(): el test pregunta si esta fuera de servicio",
        "semaforo.cpp NO tiene testLedsAdmitido(): TEST_LEDS corre en cualquier modo y su "
        "fase verde sale contra el otro sentido (cinta del 28/09)")
    if admit is None:
        return
    mRet = re.search(r"return\s+(.+?);", re.sub(r"\s+", " ", admit))
    if not mRet:
        raise fw.Abortado("testLedsAdmitido() no tiene un `return <expr>;` legible")
    # El rojo del Esclavo CONFIRMADO (ACK_RED en C_MENU_IDLE) entra como una variable mas
    # de la tabla, RC, evaluada en sus dos valores: sin ella, recien entrado en MENU desde
    # AUTO el Esclavo puede seguir en verde (medido con el arnes del automatico, 28/09).
    exprAdm = re.sub(r"coordinador_rojoEsclavoConfirmado\s*\(\s*\)", "RC", mRet.group(1))
    enum = re.search(r"enum\s+ModoSistema\s*\{([^}]*)\}",
                     fw.codigo("Maestro", "include", "modos.h"))
    if not enum:
        raise fw.Abortado("no se lee el enum ModoSistema de Maestro/include/modos.h")
    modos = [t.strip() for t in enum.group(1).split(",") if t.strip()]
    main_c = fw.codigo("Maestro", "src", "main.cpp")
    setups = {k: v for k, v in
              re.findall(r"case\s+(\w+)\s*:\s*(\w+)\s*\(\s*\)\s*;\s*break", main_c)
              if v.lower().endswith("setup")}
    fuera_de_servicio = set()
    for modo, fn in setups.items():
        for f in fw.fuentes_de("Maestro", "src"):
            cf = fw.codigo("Maestro", "src", f)
            mm = re.search(r"\bvoid\s+%s\s*\(\s*\)\s*\{" % re.escape(fn), cf)
            if mm:
                tr = _bloque(cf, mm.end() - 1)
                if tr and "coordinador_forzarMenu" in cf[tr[0]:tr[1]]:
                    fuera_de_servicio.add(modo)
    if len(setups) != len(modos) or not fuera_de_servicio:
        raise fw.Abortado(
            "del switch de main.cpp salieron %d setups para %d modos y %d fuera de "
            "servicio: el lector se quedo ciego y la tabla de abajo no mediria nada"
            % (len(setups), len(modos), len(fuera_de_servicio)))
    estados = ("S_ROJO", "S_VERDE", "S_AMARILLO", "S_FALLO")
    desconocidos = set(_IDENT.findall(exprAdm)) - set(modos) - set(estados) - {
        "m", "estado", "RC"}
    if desconocidos:
        raise fw.Abortado("testLedsAdmitido() menciona %s, que este pack no sabe evaluar"
                          % ", ".join(sorted(desconocidos)))
    def _admision(expr):
        py = expr.replace("&&", " and ").replace("||", " or ").replace("!=", " __NE__ ")
        py = py.replace("!", " not ").replace("__NE__", "!=")
        malos, alguno = [], False
        for modo in modos:
            for est in estados:
                for rc in (True, False):
                    ent = {n: n for n in modos + list(estados)}
                    ent.update({"m": modo, "estado": est, "RC": rc})
                    si = bool(eval(py, {"__builtins__": {}}, ent))  # noqa: S307
                    alguno = alguno or si
                    if si and not (modo in fuera_de_servicio and est == "S_ROJO" and rc):
                        malos.append("%s/%s/%s" % (modo, est,
                                                   "rojo_esclavo" if rc else "SIN_ACK_RED"))
        return malos, alguno

    malos, alguno = _admision(exprAdm)
    # Control negativo sobre el texto REAL: la misma expresion con un modo de servicio
    # colado -y sin la condicion de la luz- tiene que salir con casos malos.
    enServicio = sorted(set(modos) - fuera_de_servicio)[0]
    b.control_negativo(
        bool(_admision(exprAdm.replace("m == MENU", "m == MENU || m == %s" % enServicio,
                                       1))[0])
        and bool(_admision(re.sub(r"&&\s*estado\s*==\s*S_ROJO", "", exprAdm))[0])
        and bool(_admision(re.sub(r"&&\s*RC\b", "", exprAdm))[0]),
        "testLedsAdmitido() con %s colado, sin `estado == S_ROJO` o sin el rojo del "
        "Esclavo confirmado, sale con casos admitidos: la tabla distingue el defecto"
        % enServicio)
    b.verificar(
        not malos,
        "el test solo se admite con los DOS postes en rojo fijo (%s, luz en S_ROJO y "
        "ACK_RED del Esclavo): en "
        "ningun modo con ciclo puede encender un verde contra el otro sentido"
        % ", ".join(sorted(fuera_de_servicio)),
        "EL TEST SE ADMITE EN SERVICIO: %s. Su fase verde enciende VERDE1/VERDE2 sin "
        "mirar el ciclo, y SFTY-2 no lo impide porque solo conoce este poste"
        % ", ".join(malos))
    b.verificar(
        alguno,
        "y hay al menos un caso admitido: el test no se ha quedado sin uso",
        "testLedsAdmitido() no admite NINGUN caso: el test de lamparas dejo de existir "
        "sin que nadie lo decidiera")
