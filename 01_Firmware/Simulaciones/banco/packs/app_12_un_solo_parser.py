# ===== banco/packs/app_12_un_solo_parser.py =====
#
# UN SOLO PARSER, Y EL QUE SE PRUEBA ES EL QUE SE INSTALA.
#
# LA PROPIEDAD, EN UNA LINEA: la operacion de partir una trama en campos esta escrita
# UNA vez, y las suites unitarias ejercen ESA y no una copia.
#
# LO QUE SE MIDIO EL 05/09, QUE ES POR LO QUE ESTE PACK EXISTE.
#
# Habia TRES implementaciones de la misma operacion -partir por ',' y luego por el
# primer ':'-, en tres ficheros, con tres contratos:
#
#   app.js  _camposNmea()        claves VERBATIM. Es la que corre en el telefono, y
#                                sirve a las cinco cabeceras que la app lee.
#   js/nmea_parser.js parseStatus()  su propio bucle + un switch que RENOMBRA a
#                                minuscula. Solo $STATUS. Sus llamadores eran las
#                                pruebas... y un arnes de la compuerta.
#   test_unitarios_app.js        una tercera, con `pair.length` en vez del primer ':',
#                                que solo ejercian sus propias 29 comprobaciones.
#
# O sea: LA QUE SE PROBABA NO ERA LA QUE SE INSTALABA. Un arreglo en el parser con
# pruebas no llegaba a la pantalla de nadie, y sus pruebas seguian en verde. Es la
# "segunda copia escrita a mano que alguien sincroniza" de CLAUDE.md 3.bis, con el
# agravante de que la copia era la que tenia la cobertura.
#
# Y NO ERA TEORICO: la regla del primer ':' -N-62, el HORA:18:25:00 que se truncaba a
# "18"- hubo que arreglarla copia por copia, y la tercera nunca se arreglo. Acertaba por
# casualidad sobre las tramas de su propia suite.
#
# POR QUE ESTE PACK Y NO OTRO MAS (CLAUDE.md 2.bis). No certifica otra vez lo ya
# certificado: contesta una pregunta que hoy no contesta nadie. documentos_03 ata la
# rama de $STATUS de app.js y el Manual 10 al C++, pero NUNCA ABRE js/nmea_parser.js;
# app_07 censa MODULOS huerfanos y NMEAParser tiene llamador, asi que pasa. La lista de
# campos del parser era una copia del contrato del cable SIN VIGILANTE, y las tres
# implementaciones podian divergir sin que ningun instrumento se pusiera rojo.
#
# LO QUE ESTE PACK NO CIERRA, y va escrito para que no se lea como cerrado: parseStatus()
# sigue existiendo con su lista. No se puede retirar porque tiene un consumidor de
# produccion -Simulaciones/simulador_app_bluetooth.py, que corre en la compuerta y compara
# campo a campo con sus nombres cortos y sus tipos-. Lo que se consigue aqui es que esa
# lista no pueda desviarse en silencio.
#
# SOBRE LAS ETIQUETAS SFTY: ninguna. No ejerce ni un umbral ni una maniobra; mide que dos
# ficheros de la app no digan cosas distintas del mismo cable. Figurar en la tabla de
# trazabilidad sin ejercer la regla es peor que una fila vacia.

import re

NOMBRE = "app_12_un_solo_parser"
DESCRIPCION = "una sola funcion parte las tramas, y las suites unitarias ejercen ESA"

PUNTAS = ("Maestro", "Esclavo")

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
PARSER_JS = ("05_Funcional", "App_Semaforo", "js", "nmea_parser.js")
SUITE_TDD = ("05_Funcional", "App_Semaforo", "tests", "test_unitarios.js")
SUITE_APP = ("05_Funcional", "App_Semaforo", "test_unitarios_app.js")

# El nombre del partidor unico. Va aqui arriba y no incrustado en cinco expresiones
# regulares: si alguien lo renombra, este pack ABORTA en el bloque 1 en vez de comparar
# contra un fichero que ya no lo tiene y aprobar por no encontrar nada.
PARTIDOR = "camposDeTrama"

# La huella de partir un campo por su primer ':'. Es LA operacion que no puede estar
# escrita dos veces. Se buscan las dos formas con las que se ha escrito en este
# repositorio -indexOf(':') y split(':')-, porque la tercera copia usaba la segunda.
_HUELLA_PARTIR = re.compile(r"\.indexOf\(':'\)|\.split\(':'\)")


def _cuerpo(js, nombre):
    """El cuerpo de una funcion, por conteo de llaves.

    No con un `.*?}` perezoso: se cortaria en la primera llave interna y devolveria
    medio cuerpo, que es un censo que aprueba casi todo."""
    m = re.search(r"(?:function\s+)?%s\s*\([^)]*\)\s*\{" % re.escape(nombre), js)
    if not m:
        return None
    ini = m.end() - 1
    prof = 0
    for j in range(ini, len(js)):
        if js[j] == "{":
            prof += 1
        elif js[j] == "}":
            prof -= 1
            if prof == 0:
                return js[ini + 1:j]
    return None


def _sin_comentarios(js):
    """El codigo sin prosa. Sin esto, un fichero que DOCUMENTA haber quitado un bucle
    -y este los documenta, largo- se acusaria de conservarlo: la huella esta en el
    comentario que explica la mudanza. Es CLAUDE.md 4 sobre el propio instrumento."""
    js = re.sub(r"/\*[\s\S]*?\*/", "", js)
    return re.sub(r"^\s*//.*$", "", js, flags=re.M)


def _campos_del_cpp(fw, punta):
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    m = re.search(r'"(\$STATUS,[^"]*)"', codigo)
    if m is None:
        return None
    partes = [p for p in m.group(1).split(",") if p]
    return [p.split(":")[0].strip() for p in partes[1:]]


def _lista(js, nombre):
    """Los literales de un `const NOMBRE = [ ... ];`."""
    m = re.search(r"const\s+%s\s*=\s*\[(.*?)\];" % re.escape(nombre), js, re.S)
    if m is None:
        return None
    return [x for x in re.findall(r"'([^']*)'", m.group(1))]


def correr(b, fw):
    app = fw.texto_repo(*APP_JS)
    parser = fw.texto_repo(*PARSER_JS)
    app_codigo = _sin_comentarios(app)
    parser_codigo = _sin_comentarios(parser)

    # -------------------------------------------------------------------------
    b.titulo("1. Existe UN partidor, y esta donde se puede compartir")
    # -------------------------------------------------------------------------
    cuerpo_partidor = _cuerpo(parser_codigo, PARTIDOR)
    if cuerpo_partidor is None:
        raise fw.Abortado(
            "no se halla %s() en js/nmea_parser.js. O se renombro o se movio, y en los "
            "dos casos las comprobaciones de abajo compararian contra nada -que es como "
            "un pack aprueba una app que ya no existe (CLAUDE.md 5)-" % PARTIDOR)

    b.verificar(
        bool(_HUELLA_PARTIR.search(cuerpo_partidor)),
        "%s() es quien parte de verdad: su cuerpo lleva la operacion del primer ':'"
        % PARTIDOR,
        "%s() existe pero NO parte nada: su cuerpo no tiene ni indexOf(':') ni "
        "split(':'). Un partidor que no parte deja a los de abajo comparando contra un "
        "cascaron, y todo este pack pasaria midiendo el nombre de una funcion"
        % PARTIDOR)

    # -------------------------------------------------------------------------
    b.titulo("2. Y es el UNICO: nadie mas parte una trama por su cuenta")
    # -------------------------------------------------------------------------
    # En el parser: una sola huella, la de PARTIDOR. parseStatus, parseAlarm y parseError
    # tienen que ENTRAR por el, no traer su propio bucle.
    huellas_parser = _HUELLA_PARTIR.findall(parser_codigo)
    b.verificar(
        len(huellas_parser) == 1,
        "js/nmea_parser.js parte por el primer ':' en UN solo sitio, dentro de %s()"
        % PARTIDOR,
        "js/nmea_parser.js tiene %d sitios que parten un campo por ':' y solo puede "
        "tener uno. Cada copia de mas es una regla que alguien va a arreglar en un sitio "
        "y no en el otro: la del primer ':' (N-62) ya se pago asi"
        % len(huellas_parser))

    for nombre_fn in ("parseStatus", "parseAlarm", "parseError"):
        cuerpo = _cuerpo(parser_codigo, nombre_fn)
        b.verificar(
            cuerpo is not None and PARTIDOR in cuerpo,
            "%s() entra por %s() en vez de partir la trama otra vez"
            % (nombre_fn, PARTIDOR),
            "%s() no llama a %s(): tiene su propio criterio para separar clave de valor. "
            "Dos criterios sobre el mismo cable dan resultados distintos el dia que un "
            "valor lleve un ':' de mas, y solo uno de los dos se arregla"
            % (nombre_fn, PARTIDOR))

    # En la app: _camposNmea() delega. Se mide su CUERPO y no el fichero entero, porque
    # app.js parte por ':' en otro sitio con todo el derecho -el atributo data-cmd de un
    # boton, 'SET_MODO:AUTO'-, que no es una trama. Un censo sobre el fichero entero
    # acusaria de duplicar el parser a quien lee un atributo del HTML.
    cuerpo_app = _cuerpo(app_codigo, "_camposNmea")
    if cuerpo_app is None:
        raise fw.Abortado(
            "no se halla _camposNmea() en app.js: es la puerta por la que las cinco "
            "cabeceras que la app lee parten su trama. Sin ella no se puede decir si la "
            "app usa el partidor comun o el suyo")
    b.verificar(
        PARTIDOR in cuerpo_app and not _HUELLA_PARTIR.search(cuerpo_app),
        "_camposNmea() de app.js DELEGA en %s(): la app no lleva su propio partidor"
        % PARTIDOR,
        "_camposNmea() vuelve a partir la trama por su cuenta (llama a %s: %s | tiene "
        "huella propia: %s). Es el defecto original: dos parsers, dos convenios, y el "
        "que tiene pruebas no es el que se instala"
        % (PARTIDOR, PARTIDOR in cuerpo_app,
           bool(_HUELLA_PARTIR.search(cuerpo_app))))

    # -------------------------------------------------------------------------
    b.titulo("3. La que se PRUEBA es la que se USA")
    # -------------------------------------------------------------------------
    # Esta es la propiedad y no el sintoma. Que exista un solo partidor no sirve de nada
    # si las suites siguen ejerciendo otra cosa: era exactamente la situacion de partida
    # -29 comprobaciones en verde sobre codigo que no viaja en la APK-.
    suites = {}
    for etiqueta, partes in (("tests/test_unitarios.js", SUITE_TDD),
                             ("test_unitarios_app.js", SUITE_APP)):
        try:
            suites[etiqueta] = fw.texto_repo(*partes)
        except Exception:
            raise fw.Abortado(
                "no se pudo leer %s. Las dos suites unitarias de la app son lo unico "
                "que ejerce el parser fuera del navegador; sin ellas esto no mide si lo "
                "probado es lo instalado" % etiqueta)

    ejercen = [e for e, t in suites.items()
               if re.search(r"NMEAParser\.%s\(" % PARTIDOR, t)
               or re.search(r"\b%s\(" % PARTIDOR, _sin_comentarios(t))]
    b.verificar(
        bool(ejercen),
        "%d de las %d suites unitarias ejercen %s(), la funcion que corre en el telefono "
        "(%s)" % (len(ejercen), len(suites), PARTIDOR, ", ".join(sorted(ejercen))),
        "NINGUNA de las dos suites unitarias llama a %s(). Se estaria probando otra cosa "
        "distinta de la que se instala, que es de donde venimos: un arreglo en el parser "
        "probado no llegaba a la pantalla de nadie y sus pruebas seguian verdes"
        % PARTIDOR)

    # Y la tercera copia, por su huella: la suite que se reimplementaba el parser dentro.
    propia = _HUELLA_PARTIR.findall(_sin_comentarios(suites["test_unitarios_app.js"]))
    b.verificar(
        not propia,
        "test_unitarios_app.js ya no lleva su propio partidor: importa el de la app",
        "test_unitarios_app.js vuelve a partir las tramas con codigo suyo (%d huella(s)). "
        "Es la tercera copia: la que uso `pair.length` durante meses en vez del primer "
        "':' y acertaba solo sobre las tramas de su propia suite" % len(propia))

    # -------------------------------------------------------------------------
    b.titulo("4. Las dos listas de campos dicen lo mismo, y lo mismo que el C++")
    # -------------------------------------------------------------------------
    # LO QUE SE COMPARA Y POR QUE ESE ES EL BORDE CORRECTO (CLAUDE.md 4.quinquies: al
    # comparar contra un borde, escribe CUAL y por que).
    #
    # NO se compara el switch de parseStatus contra los `data.X` de la rama de $STATUS de
    # app.js, aunque sea la comparacion que parece natural. Se midio y da un falso
    # positivo: RTT sale en el switch y NO en la rama, porque app.js lo lee dentro de
    # lecturaDeEnlace(), que es otra funcion. Un pack montado sobre esa comparacion
    # acusaria al parser de tener un campo de mas que si se usa.
    #
    # El borde bueno es el mismo que usa documentos_03: EL C++. Es la unica lista que no
    # es copia de nadie, las dos direcciones significan algo, y ata la tercera pata que
    # documentos_03 deja suelta -alli se comparan el Manual 10 y app.js contra el
    # firmware; js/nmea_parser.js no lo abre nadie-.
    emitidos = {p: _campos_del_cpp(fw, p) for p in PUNTAS}
    if any(v is None for v in emitidos.values()):
        raise fw.Abortado(
            "no se hallo la trama $STATUS en bluetooth.cpp de %s: fallo el buscador o la "
            "trama se construye de otra forma. Comparar la lista del parser contra nada "
            "la aprobaria entera"
            % ", ".join(p for p, v in emitidos.items() if v is None))
    # El Maestro es el que emite el conjunto completo -el Esclavo no manda ESC porque no
    # tiene de donde sacarlo, y esa asimetria la vigila documentos_03-.
    del_cable = emitidos["Maestro"]

    cuerpo_status = _cuerpo(parser_codigo, "parseStatus")
    del_parser = re.findall(r"case '([A-Z_]+)':", cuerpo_status or "")
    if not del_parser:
        raise fw.Abortado(
            "el switch de parseStatus() no devolvio ni un campo: o cambio de forma o "
            "_cuerpo() no lo encuentra. Con la lista vacia, 'no sobra ninguno' seria "
            "cierto y no significaria nada")

    faltan = [c for c in del_cable if c not in del_parser]
    sobran = [c for c in del_parser if c not in del_cable]
    b.verificar(
        not faltan and not sobran,
        "la lista de parseStatus() y la que emite el Maestro son la misma (%d campos: %s)"
        % (len(del_parser), ", ".join(del_parser)),
        "js/nmea_parser.js y el firmware no dicen lo mismo. Faltan en el parser: %s. "
        "Sobran: %s. Un campo que falta lo DESCARTA en silencio -y el arnes "
        "simulador_app_bluetooth.py lo compara campo a campo-; uno que sobra es un "
        "contrato que nadie emite y que el siguiente que lo lea se creera"
        % (", ".join(faltan) or "-", ", ".join(sobran) or "-"))

    # -------------------------------------------------------------------------
    b.titulo("5. Un solo vocabulario de ausencia entre los dos ficheros")
    # -------------------------------------------------------------------------
    # El propio nmea_parser.js lo declara: "el MISMO que RF_NO_MEDIDO de app.js... Si
    # esta lista y la de app.js dejan de coincidir, una punta declarara la ausencia y la
    # otra la pintara como dato". Era una AFIRMACION SOBRE EL CODIGO escrita en un
    # comentario y sin comprobar por nadie, que es justo lo que CLAUDE.md 3.bis prohibe
    # para los motivos de las excepciones. Desde hoy se mide.
    del_parser_sd = _lista(parser, "_SIN_DATO")
    del_app_sd = _lista(app, "RF_NO_MEDIDO")
    if del_parser_sd is None or del_app_sd is None:
        raise fw.Abortado(
            "no se hallo _SIN_DATO en el parser (%s) o RF_NO_MEDIDO en app.js (%s): son "
            "las dos listas que dicen que significa 'no lo se' en una trama, y sin las "
            "dos no hay nada que comparar"
            % (del_parser_sd is not None, del_app_sd is not None))
    b.verificar(
        sorted(del_parser_sd) == sorted(del_app_sd),
        "el vocabulario de ausencia es el mismo en los dos ficheros (%d marcas: %s)"
        % (len(del_app_sd), ", ".join(x or "(vacio)" for x in del_app_sd)),
        "_SIN_DATO del parser y RF_NO_MEDIDO de app.js han dejado de coincidir. Parser: "
        "%s. App: %s. Una marca que solo conoce uno de los dos hace que una punta declare "
        "la ausencia y la otra la pinte como si fuera una medida -un 0%% de enlace, una "
        "bateria a cero- sin que nadie haya medido nada"
        % (sorted(del_parser_sd), sorted(del_app_sd)))

    # -------------------------------------------------------------------------
    b.titulo("6. Controles negativos: la prueba sabe fallar")
    # -------------------------------------------------------------------------
    b.control_negativo(
        bool(_HUELLA_PARTIR.search("const s = t.indexOf(':');"))
        and bool(_HUELLA_PARTIR.search("const p = t.split(':');"))
        and not _HUELLA_PARTIR.search("const p = t.split(',');"),
        "el detector de partidores reconoce las DOS formas con las que se escribio en "
        "este repositorio -indexOf(':') y split(':')- y no confunde con ellas el corte "
        "por comas, que es otra cosa")

    b.control_negativo(
        _cuerpo("function f(a) { if (a) { return 1; } return 2; }", "f")
        == " if (a) { return 1; } return 2; ",
        "_cuerpo() cuenta llaves: devuelve la funcion ENTERA y no se corta en la primera "
        "llave interna, que devolveria medio cuerpo y aprobaria casi todo")

    b.control_negativo(
        _cuerpo("function otra() { return 0; }", "_camposNmea") is None,
        "y cuando la funcion no esta devuelve None, que es lo que dispara el ABORTADO: "
        "un cuerpo vacio se leeria como 'no tiene partidor propio' y aprobaria")

    falso = _sin_comentarios("// esto explica el viejo x.indexOf(':')\nconst a = 1;")
    b.control_negativo(
        not _HUELLA_PARTIR.search(falso)
        and bool(_HUELLA_PARTIR.search(_sin_comentarios("const s = x.indexOf(':');"))),
        "la prosa no cuenta como codigo: un comentario que CITA el bucle retirado no "
        "acusa al fichero de conservarlo, y uno de verdad si se ve")

    b.control_negativo(
        _campos_del_cpp(fw, "Maestro") != ["NODE", "MODO"],
        "el censo de campos del C++ no devuelve una lista corta inventada: si lo hiciera, "
        "'no falta ninguno' seria cierto sobre dos campos y no sobre la trama entera")

    b.reportar(
        "LO QUE ESTE PACK NO MIDE",
        ["- Que parseStatus() TRADUZCA bien. Comprueba que su lista de campos sea la del",
         "  cable; que 'restante' salga de T y no de RTT lo ejerce",
         "  simulador_app_bluetooth.py, que compara campo a campo contra el micro modelado.",
         "- El GENERADOR. buildCommand() de test_unitarios_app.js sigue siendo una copia a",
         "  mano de NMEAParser.generarComando(), con el mismo contrato. No se unifico hoy",
         "  porque generarComando() tiene CERO llamadores en app.js a proposito -la app",
         "  compone la trama desnuda, sin checksum- y tocarlo mueve la pregunta abierta del",
         "  *XX que vigila simulador_puente_esp32.py. Queda medido y sin cerrar.",
         "- Los RANGOS de validateTiempos() en test_unitarios_app.js siguen en 1..15 min",
         "  mientras el C++ y app.js estan en 3..15 (app_11). No falla hoy porque ninguno",
         "  de sus siete casos toca el borde -no prueba ni 1 ni 2-, o sea que es una copia",
         "  vieja que NO PUEDE fallar: la peor clase. Se reporta, no se arregla desde aqui.",
         "- Que parseStatus() deba EXISTIR. Tiene consumidor de produccion en la compuerta",
         "  (simulador_app_bluetooth.py), asi que retirarla es una decision que se toma",
         "  con ese arnes delante y no desde la app."])
