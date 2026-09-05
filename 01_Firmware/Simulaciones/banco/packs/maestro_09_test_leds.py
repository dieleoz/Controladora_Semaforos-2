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

# Las funciones que pueden llamar a escribirPines() sin pasar por aplicarSalidas().
# Es un TRINQUETE, no un absoluto: son los cuatro caminos de la senal del mando, que
# interceptan la salida a proposito. Una funcion NUEVA en esta lista es un camino
# nuevo a los pines y tiene que discutirse, no colarse.
INTERCEPTAN_SFTY21 = {
    "aplicarSalidas",          # el camino bueno: la barrera esta dentro
    "terminarSenal",           # vuelca ultR/ultA/ultV, ya saneados por el enclavamiento
    "actualizarSenal",         # destellos y ambar rapido: nunca verde
    "semaforo_destellosRojos",  # hueco inicial del primer destello
    "semaforo_ambarRapido",    # ambar de rechazo
}

# El tercer argumento de escribirPines() que NO es un verde crudo. `false` es apagado;
# `ultV` es el verde que el enclavamiento ya decidio y guardo.
VERDE_ADMITIDO = ("false", "ultV")

_DEF = re.compile(
    r"^(?:static\s+)?(?:void|bool|uint8_t|unsigned\s+long|const\s+char\s*\*|"
    r"EstadoSemaforo)\s+(\w+)\s*\([^)]*\)\s*\{", re.M)


def _bloque(codigo, i):
    """[inicio, fin] del bloque que abre en codigo[i] == '{'. None si no cierra."""
    nivel = 0
    for j in range(i, len(codigo)):
        if codigo[j] == "{":
            nivel += 1
        elif codigo[j] == "}":
            nivel -= 1
            if nivel == 0:
                return (i, j + 1)
    return None


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


def _sin_asignacion(expr, codigo):
    """Desenvuelve la condicion cuando viene dentro de una asignacion a la bandera.

    N-153. La orden de la pluma pasa a ser

        digitalWrite(MOTOR_TALANQUERA,
                     (plumaAbierta = ((verde && !testLedsActivo) || estado == S_FALLO))
                         ? TALANQUERA_ABRIR : TALANQUERA_CERRAR);

    porque el $STATUS publica ahora esa posicion y el valor tiene que salir del MISMO
    parentesis que mueve el pin: una copia de la formula en la linea de al lado seria la
    que se queda vieja el dia que la condicion cambie.

    LA EXCEPCION SE MIDE, NO SE ESCRIBE (CLAUDE.md 3.bis). No se acepta cualquier
    asignacion: solo la que asigna a la bandera que devuelve semaforo_plumaArriba(),
    leida del propio fuente. Con cualquier otro identificador esto devuelve la expresion
    tal cual y el pack ABORTA como hacia antes -que es lo correcto: una expresion que no
    se entiende no se aprueba-."""
    m = re.match(r"^\(\s*([A-Za-z_]\w*)\s*=\s*(.+)\)$", expr.strip())
    if not m:
        return expr
    cuerpo = _cuerpo(codigo, "semaforo_plumaArriba")
    publica = re.search(r"return\s+([A-Za-z_]\w*)\s*;", cuerpo or "")
    if not publica or publica.group(1) != m.group(1):
        return expr
    return m.group(2).strip()


def _evaluar_pluma(cond, bandera, verde, test, fallo):
    """Evalua la condicion REAL del C++ con la tabla de verdad dada.

    No se reescribe la logica en Python -eso seria una segunda copia que alguien
    tendria que sincronizar, que es el defecto que este banco persigue-: se traduce
    la expresion y se evalua tal cual esta escrita en el fuente."""
    py = re.sub(r"estado\s*==\s*S_FALLO", "ES_FALLO", cond)
    py = py.replace("&&", " and ").replace("||", " or ").replace("!", " not ")
    return bool(eval(py, {"__builtins__": {}},  # noqa: S307
                     {"verde": verde, bandera: test, "ES_FALLO": fallo}))


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

    b.verificar(
        llamadas >= 6,
        "censadas %d llamadas a escribirPines() en semaforo.cpp, clasificadas por la "
        "funcion que las contiene" % llamadas,
        "solo %d llamadas a escribirPines() halladas. Un censo casi vacio aprobaria el "
        "fichero sin haber mirado un solo camino a los pines" % llamadas)

    b.verificar(
        not crudos,
        "ninguna llamada directa a escribirPines() mete un verde crudo: el unico verde "
        "que llega a los pines es el que el enclavamiento SFTY-2 ya saneo",
        "VERDE FUERA DEL ENCLAVAMIENTO: %s. Ese verde llega a la lampara sin que "
        "aplicarSalidas() lo haya visto, asi que SFTY-2 no lo ha enclavado y "
        "ultR/ultA/ultV no lo conocen -una senal del mando que terminara ahi volcaria "
        "una foto vieja-. Y como la talanquera cuelga del mismo argumento, ademas abre "
        "la barrera" % ", ".join(crudos))

    # ---- 4. Quien llama directamente es SOLO la senal del mando ----
    directas = set()
    for mc in re.finditer(r"\bescribirPines\s*\(", codigo):
        quien = _quien_contiene(funciones, mc.start())
        if quien:
            directas.add(quien)
    intrusas = sorted(directas - INTERCEPTAN_SFTY21)
    b.verificar(
        not intrusas,
        "las %d funciones que llaman a escribirPines() son las conocidas: "
        "aplicarSalidas y los cuatro caminos de la senal SFTY-21" % len(directas),
        "CAMINO NUEVO A LOS PINES: %s llama(n) a escribirPines() directamente. Puede "
        "ser legitimo -la senal del mando lo es- pero nadie lo ha mirado: si no pasa "
        "por aplicarSalidas(), la barrera no lo cubre" % ", ".join(intrusas))

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

    # ---- 6. Con una senal del mando en curso, el test NO corre por debajo ----
    #
    # Con senalActiva, aplicarSalidas() guarda y no escribe: el test gastaria sus
    # segundos sin encender una lampara y eso se lee como lamparas fundidas. Y peor:
    # el return del bloque dejaria actualizarSenal() sin llamar y la senal no
    # terminaria nunca.
    j = bloqueTest.find("senalActiva")
    b.verificar(
        j >= 0 and j < bloqueTest.find("aplicarSalidas"),
        "el bloque del test mira senalActiva ANTES de pedir ninguna salida: con la "
        "senal del mando ocupando las luces, el test espera en vez de gastarse a "
        "oscuras",
        "el bloque del test no consulta senalActiva antes de llamar a aplicarSalidas(). "
        "Con una senal en curso aplicarSalidas() guarda y NO escribe: seis segundos de "
        "test sin encender una lampara, que un tecnico lee como tres lamparas fundidas")

    ramaSenal = None
    k = bloqueTest.find("if (senalActiva)")
    if k >= 0:
        t = _bloque(bloqueTest, bloqueTest.index("{", k))
        ramaSenal = bloqueTest[t[0]:t[1]]
    b.verificar(
        ramaSenal is not None and "return" not in ramaSenal
        and "aplicarSalidas" not in ramaSenal,
        "y en esa rama no hay return ni salida: se cae hasta actualizarSenal(), de modo "
        "que la senal termina y las luces vuelven",
        "la rama de senalActiva del test devuelve o escribe salidas. Si devuelve, "
        "actualizarSenal() no se llama: la senal no termina NUNCA, senalActiva se queda "
        "en true y aplicarSalidas() no vuelve a escribir un pin en toda la vida del "
        "equipo")

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
    expr = _sin_asignacion(expr, codigo)

    desconocidos = sorted(set(_IDENT.findall(expr)) -
                          {"verde", bandera, "estado", "S_FALLO"})
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
        not pluma(False, False, False) and not pluma(False, True, False),
        "sin verde y sin fallo la pluma esta abajo, haya test o no",
        "la pluma abre sin verde y sin S_FALLO. Es la direccion peligrosa: una barrera "
        "levantada invitando a pasar con la luz en rojo")

    b.verificar(
        pluma(False, False, True) and pluma(False, True, True),
        "en S_FALLO la pluma sigue subiendo -la politica de SFTY-6 que eligio el "
        "cliente el 27/08- y el test no se la lleva por delante",
        "la excepcion de S_FALLO se perdio: con el equipo sin enlace la barrera se "
        "quedaria ABAJO, cerrando la via por completo, que es la politica CONTRARIA a "
        "la decidida")

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

    b.control_negativo(
        _condicion_pluma("{ digitalWrite(MOTOR_TALANQUERA, (a && b) ? X : Y); }")
        == ("(a && b)", "X", "Y"),
        "el extractor del ternario devuelve la condicion entera y sus dos ramas, y no "
        "se queda con un trozo")
