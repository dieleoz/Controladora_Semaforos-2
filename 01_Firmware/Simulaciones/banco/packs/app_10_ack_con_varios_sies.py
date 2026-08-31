# ===== banco/packs/app_10_ack_con_varios_sies.py =====
#
# UNA ORDEN CON VARIOS SIES NO SE PINTA CON UN SOLO TEXTO.
#
# LA PROPIEDAD, EN UNA LINEA: si el C++ puede contestar a un comando con mas de un
# RESULT distinto, la app tiene un texto DISTINTO para cada uno.
#
# POR QUE, Y ES UN DEFECTO DE CALLE, NO DE ESTILO
#
# El firmware de este proyecto se toma un trabajo enorme en no mentir en el acuse: en
# vez de un OK universal, inventa un literal por cada final posible, y sus comentarios
# explican uno a uno por que no vale contestar OK. Muestra:
#
#   "$ACK,CMD:AMBAR_EMERGENCIA,RESULT:SALIENDO_TODO_ROJO"
#      -> "El RESULT no es OK a proposito: el ambar no esta puesto todavia y decir OK
#          seria dar por hecho un cambio de luz que tarda hasta 90 s."
#
# Y la app cogia esos literales y los pintaba TODOS igual:
#
#   addEvent('green', 'Equipo: orden [' + cual + '] ACEPTADA (' + data.RESULT + ')');
#
# En verde, con la palabra ACEPTADA delante y el literal entre parentesis. Es la barrera
# del $ACK que no mira (CLAUDE.md 6) con las puntas cambiadas: alli el que mentia era el
# micro, aqui el telefono. El coste esta medido caso a caso:
#
#   RETIRADO_QUEDA_MANDO       queda OTRO ambar puesto desde el mando, y ese NO se quita
#                              por radio. Pintado como un si, el tecnico se va del poste
#                              esperando un cambio de fase que no llegara nunca.
#   SALIENDO_TODO_ROJO         lo pedido tarda hasta 90 s en ocurrir.
#   HORA_PUESTA_SIN_PROPAGAR   la hora entro en el Maestro y NO llego al Esclavo -y de
#                              esa hora cuelga la autorizacion del Modo Degradado-.
#
# LAS DOS DIRECCIONES, QUE NO SIGNIFICAN LO MISMO
#
#   falta en la app  ->  el equipo contesta algo que la pantalla no sabe distinguir, y
#                        lo pinta con el texto generico: un "aceptada" sobre algo que no
#                        ha pasado.
#   sobra en la app  ->  la tabla nombra una respuesta que el firmware ya no manda. No
#                        hace dano hoy, pero es una tabla que se esta quedando vieja
#                        sin avisar, y manana alguien la leera como el contrato.
#
# POR QUE SOLO LOS $ACK Y NO LOS $ERR. Un rechazo se lee como un rechazo lleve el texto
# que lleve: el literal en crudo es feo, pero no le hace creer a nadie que la orden
# entro. Un ACUSE es lo contrario: se lee como exito por el hecho de serlo, y ahi la
# diferencia entre dos literales es la diferencia entre irse del poste y quedarse. Los
# $ERR se vigilan en la otra direccion -que la tabla de la app no nombre motivos que el
# firmware ya no manda-, que es lo que si puede quedarse viejo.
#
# SOBRE LAS ETIQUETAS SFTY: ninguna. Roza SFTY-6 -SALIENDO_TODO_ROJO sale de la salida
# del Degradado- pero no la ejerce: no comprueba ni un umbral ni una maniobra, comprueba
# que la pantalla no junte dos respuestas distintas. Figurar en la tabla de trazabilidad
# sin ejercer la regla es peor que una fila vacia.

import re

NOMBRE = "app_10_ack_con_varios_sies"
DESCRIPCION = "toda orden con mas de un RESULT posible tiene un texto distinto por respuesta en la app"

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
BT_MAESTRO = ("Maestro", "src", "bluetooth.cpp")
BT_ESCLAVO = ("Esclavo", "src", "bluetooth.cpp")

# Las dos tablas de la app, por el nombre con el que estan declaradas. No es una lista
# de textos escrita a mano: es la DIRECCION donde vive la tabla, igual que los packs del
# firmware direccionan un .cpp por su tupla de ruta. Si se renombran, esto ABORTA en vez
# de comparar contra un diccionario vacio y aprobar.
TABLA_ACK = "ACK_TEXTO"
TABLA_ERR = "ERR_TEXTO"

_ACK_CPP = re.compile(r'"\$ACK,CMD:([A-Z_:0-9]+),RESULT:([A-Z_0-9]+)"')
_ERR_CPP = re.compile(r'"\$ERR,CMD:([A-Z_:0-9]+),DESC:([A-Z_0-9]+)"')


def _tabla(js, nombre):
    """{clave: {campo: texto}} de una tabla `const NOMBRE = { 'k': {...}, ... };`.

    Se lee el bloque entero con conteo de llaves y no con una expresion regular
    perezosa: los textos llevan llaves dentro de ningun sitio hoy, pero un `.*?}` se
    corta en la PRIMERA entrada y devolveria una tabla de un elemento -que es un censo
    que aprueba casi todo-."""
    m = re.search(r"const\s+%s\s*=\s*\{" % re.escape(nombre), js)
    if not m:
        return None
    i = m.end() - 1
    prof = 0
    fin = None
    for j in range(i, len(js)):
        if js[j] == "{":
            prof += 1
        elif js[j] == "}":
            prof -= 1
            if prof == 0:
                fin = j
                break
    if fin is None:
        return None
    cuerpo = js[i + 1:fin]
    fuera = {}
    for me in re.finditer(r"'([^']+)'\s*:\s*\{", cuerpo):
        k = me.group(1)
        prof2 = 0
        ini = me.end() - 1
        for j in range(ini, len(cuerpo)):
            if cuerpo[j] == "{":
                prof2 += 1
            elif cuerpo[j] == "}":
                prof2 -= 1
                if prof2 == 0:
                    fuera[k] = cuerpo[ini + 1:j]
                    break
    return fuera


def _texto_visible(bloque):
    """Lo que el operario acabaria leyendo EN EL REGISTRO: el campo `texto` entero.

    Se juntan los trozos porque el mensaje va partido en varias lineas con '+', y lo que
    hay que comparar entre dos respuestas es el mensaje completo: dos textos que
    empiezan igual y acaban distinto SI se distinguen.

    Y SE LEE SOLO EL CAMPO `texto`, NO TODOS LOS LITERALES DEL BLOQUE. Una inyeccion lo
    enseno: al copiar el mensaje de una respuesta en la otra, este pack seguia en verde
    porque los bloques todavia se diferenciaban en el `toast` -y el toast es un aviso
    que se va solo a los tres segundos-. Lo que queda escrito en el registro de eventos,
    que es lo que el tecnico lee cuando ya se ha ido del poste, era identico."""
    m = re.search(r"\btexto\s*:\s*((?:\s*'[^']*'\s*\+?)+)", bloque)
    if not m:
        return ""
    return " ".join(re.findall(r"'([^']*)'", m.group(1))).strip()


def correr(b, fw):
    js = fw.texto_repo(*APP_JS)

    # -------------------------------------------------------------------------
    b.titulo("1. Que puede contestar el C++, recalculado de las dos puntas")
    # -------------------------------------------------------------------------
    acks = {}
    errs = {}
    for partes in (BT_MAESTRO, BT_ESCLAVO):
        codigo = fw.codigo(*partes)
        for cmd, res in _ACK_CPP.findall(codigo):
            acks.setdefault(cmd, set()).add(res)
        for cmd, desc in _ERR_CPP.findall(codigo):
            errs.setdefault(cmd, set()).add(desc)

    if len(acks) < 5:
        raise fw.Abortado(
            "el censo de $ACK del C++ dio %d comando(s). Las dos puntas acusan mas de "
            "una docena desde V8.4: o el literal cambio de forma o el buscador se quedo "
            "atras, y con un censo asi este pack no exigiria nada" % len(acks))

    varios = {c: sorted(r) for c, r in acks.items() if len(r) > 1}
    b.verificar(
        bool(varios),
        "el C++ tiene %d orden(es) con mas de un RESULT posible: %s"
        % (len(varios), {c: r for c, r in varios.items()}),
        "ninguna orden del C++ tiene mas de un RESULT. Este pack existe justo para esas: "
        "si no hay ninguna, o el firmware perdio sus acuses distinguidos o el censo no "
        "sabe leerlos, y en los dos casos las comprobaciones de abajo no valen nada")

    # -------------------------------------------------------------------------
    b.titulo("2. La app tiene un texto propio para cada uno de esos sies")
    # -------------------------------------------------------------------------
    tabla = _tabla(js, TABLA_ACK)
    if tabla is None:
        raise fw.Abortado(
            "no se hallo la tabla %s en app.js. Es donde vive la traduccion de cada "
            "acuse; sin ella este pack compararia contra un diccionario vacio -y es "
            "como el banco entero se quedo en ABORTADO en N-75, con la app entrando sin "
            "vigilancia detras-" % TABLA_ACK)

    faltan = []
    for cmd, resultados in sorted(varios.items()):
        for res in resultados:
            if ("%s|%s" % (cmd, res)) not in tabla:
                faltan.append("%s|%s" % (cmd, res))
    b.verificar(
        not faltan,
        "las %d respuestas de las ordenes con varios sies tienen entrada propia en %s"
        % (sum(len(r) for r in varios.values()), TABLA_ACK),
        "%s no estan en %s: la app las pintaria con el texto generico -verde, con la "
        "palabra ACEPTADA delante-. Y ese generico es exactamente el defecto: el "
        "firmware se tomo el trabajo de inventar un literal por cada final para que no "
        "se confundieran" % (faltan, TABLA_ACK))

    # -------------------------------------------------------------------------
    b.titulo("3. Y esos textos son DISTINTOS entre si, no solo distintas entradas")
    # -------------------------------------------------------------------------
    # Sin esto, la tabla se podria rellenar copiando el mismo texto en las cuatro
    # entradas y este pack saldria verde midiendo nada: es la prueba muerta de N-51 con
    # forma de diccionario.
    for cmd, resultados in sorted(varios.items()):
        textos = {}
        for res in resultados:
            clave = "%s|%s" % (cmd, res)
            if clave in tabla:
                textos[res] = _texto_visible(tabla[clave])
        distintos = len(set(textos.values()))
        b.verificar(
            distintos == len(textos) and all(textos.values()),
            "%s: sus %d respuestas se leen distintas en pantalla" % (cmd, len(textos)),
            "%s: %d de sus %d respuestas comparten texto (o alguno esta vacio). Una "
            "tabla con la misma frase repetida distingue en el codigo y no en la "
            "pantalla, que es el unico sitio donde importa" % (cmd, len(textos) - distintos + 1, len(textos)))

    # -------------------------------------------------------------------------
    b.titulo("4. Y ninguna entrada de la tabla nombra algo que el C++ ya no manda")
    # -------------------------------------------------------------------------
    for nombre, tab, censo, campo in ((TABLA_ACK, tabla, acks, "RESULT"),
                                      (TABLA_ERR, _tabla(js, TABLA_ERR), errs, "DESC")):
        if tab is None:
            raise fw.Abortado(
                "no se hallo la tabla %s en app.js" % nombre)
        huecas = []
        for clave in sorted(tab):
            cmd, _, valor = clave.partition("|")
            if valor not in censo.get(cmd, set()):
                huecas.append(clave)
        b.verificar(
            not huecas,
            "las %d entradas de %s corresponden a respuestas que el C++ manda hoy"
            % (len(tab), nombre),
            "%s nombra %s y ninguna punta emite ese %s. La traduccion se quedo atras: no "
            "hace dano hoy, pero manana alguien leera esa tabla como el contrato del "
            "cable" % (nombre, huecas, campo))

    # -------------------------------------------------------------------------
    b.titulo("5. El camino generico sigue existiendo: un literal nuevo NO desaparece")
    # -------------------------------------------------------------------------
    # La tabla no puede ser la unica salida. El dia que el firmware anada un RESULT que
    # nadie ha traducido todavia, lo que tiene que pasar es que se vea en crudo y este
    # pack se ponga rojo -no que la pantalla se quede muda-. Un `if (dicho)` sin `else`
    # seria un $ACK que la app se traga, que es el defecto de N-75 otra vez.
    m = re.search(r"const\s+dicho\s*=\s*%s\s*\[" % TABLA_ACK, js)
    b.verificar(
        m is not None and re.search(r"\}\s*else\s*\{[^}]*ACEPTADA", js) is not None,
        "el despachador de $ACK conserva su rama generica: un RESULT sin traducir se "
        "sigue viendo en crudo en vez de perderse",
        "no hay rama generica detras de la tabla de acuses. Un literal nuevo del "
        "firmware no llegaria a la pantalla, y un $ACK que la app se traga es el defecto "
        "que N-75 cerro: el equipo contesta y el operario no se entera")

    # -------------------------------------------------------------------------
    b.titulo("6. Controles negativos: la prueba sabe fallar")
    # -------------------------------------------------------------------------
    # OJO CON EL CONTROL NEGATIVO DE ESTE: los dos bloques llevan el mismo `texto` y
    # DISTINTO `toast` a proposito. Es la forma exacta que tenia la inyeccion que dejo
    # pasar a la primera version de este pack -comparaba todos los literales del bloque,
    # asi que un toast distinto bastaba para darlos por distinguidos-. Lo que importa es
    # lo que queda escrito en el registro, no el aviso que se va solo.
    tabla_falsa = _tabla(
        "const ACK_TEXTO = {\n"
        "  'A|OK': { tono: 'green', texto: 'lo mismo', toast: 'aviso uno' },\n"
        "  'A|PENDIENTE': { tono: 'green', texto: 'lo mismo', toast: 'aviso dos' }\n"
        "};\n", "ACK_TEXTO")
    b.control_negativo(
        tabla_falsa is not None and len(tabla_falsa) == 2
        and len({_texto_visible(v) for v in tabla_falsa.values()}) == 1,
        "dos respuestas con el MISMO texto y distinto toast se detectan igual: se mira "
        "lo que queda en el registro, no el aviso que se va solo")

    tabla_corta = _tabla(
        "const ACK_TEXTO = {\n"
        "  'A|OK': { tono: 'green', texto: 'uno', toast: 'x' },\n"
        "  'A|PENDIENTE': { tono: 'red', texto: 'otro', toast: 'y' }\n"
        "};\n", "ACK_TEXTO")
    b.control_negativo(
        tabla_corta is not None and len(tabla_corta) == 2
        and len({_texto_visible(v) for v in tabla_corta.values()}) == 2,
        "y dos textos de verdad distintos NO se marcan: el detector distingue la tabla "
        "buena de la copiada, en vez de acusar a todas")

    b.control_negativo(
        len(_ACK_CPP.findall(
            'enviarTramaConCrc("$ACK,CMD:CANCELAR_AMBAR,RESULT:RETIRADO_QUEDA_MANDO");')) == 1,
        "el censo del C++ reconoce un acuse con su RESULT dentro de la llamada real")
    b.control_negativo(
        not _ACK_CPP.findall('enviarTramaConCrc("$ERR,CMD:CANCELAR_AMBAR,DESC:NO_HAY");'),
        "y no confunde un rechazo con un acuse")

    b.reportar(
        "LO QUE ESTE PACK NO MIDE",
        ["- Que el texto sea CIERTO. Comprueba que haya uno y que sea distinto de los",
         "  demas; que describa lo que de verdad hace esa rama del C++ lo tiene que leer",
         "  una persona. Un texto distinto y equivocado pasa esta prueba.",
         "- Los $ERR en la direccion de 'falta': un rechazo se lee como rechazo lleve el",
         "  texto que lleve. Solo se vigila que la tabla no se quede vieja.",
         "- Ordenes con un solo RESULT: ahi el generico basta y exigir tabla seria",
         "  obligar a escribir %d textos que no distinguen nada." % (len(acks) - len(varios))])
