# ===== banco/packs/app_08_enrutado_por_punta.py =====
#
# EL REPARTO DE COMANDOS ENTRE LAS DOS PUNTAS SE RECALCULA DEL C++, NO SE MANTIENE A MANO.
#
# EL HUECO QUE TAPA, MEDIDO, Y POR QUE NINGUN PACK PODIA VERLO.
#
# app_01_comandos cruza los comandos del .js contra el C++ pero UNE LAS DOS PUNTAS:
#
#     todos = atiende["Maestro"] | atiende["Esclavo"]
#
# Con esa union, mandarle SOLICITAR_PASO al Maestro o SET_TIEMPOS al Esclavo le da
# exactamente igual: los dos estan en `todos`. O sea que el enrutado -que es lo unico
# que decide si el boton funciona o contesta $ERR,CMD:DESCONOCIDO- no lo vigilaba nadie.
#
# Y el reparto existe, escrito a mano, en dos listas de JavaScript:
#
#     const SOLO_MAESTRO = ['SET_MODO', 'MANUAL:CAMBIAR_TURNO', ...];
#     const SOLO_ESCLAVO = ['SOLICITAR_PASO', 'AMBAR_EMERGENCIA'];
#
# Verificadas una a una el 31/08: HOY SON CORRECTAS. Eso no es un motivo para no
# vigilarlas, es el motivo exacto por el que hay que hacerlo ahora. Son una SEGUNDA COPIA
# del contrato que alguien tendra que sincronizar cada vez que una rama del despachador
# cambie de punta, y sincronizar a mano ya fallo tres veces en este repositorio -N-36,
# N-39 y la propia compuerta-. Una lista correcta y sin vigilante no es una lista
# correcta: es una que todavia no se ha quedado atras.
#
# LO QUE ESTE PACK TIENE QUE SABER DISTINGUIR, Y ES LA MITAD DEL TRABAJO.
#
# No todo lo que una punta no hace es un hueco. Hay ASIMETRIA DELIBERADA Y DOCUMENTADA:
#
#   - TEST_LEDS lo acepta el Maestro y el Esclavo LO RECHAZA NOMBRANDO EL MOTIVO
#     ("NO_EN_SERVICIO_USE_EL_MAESTRO"): encender 6 s de secuencia con verde en un
#     Esclavo en servicio mete dos vehiculos de frente en el tramo.
#   - FORZAR_ROJO esta RENOMBRADO en el Esclavo (N-83): alli promete rojo y hacia ambar
#     con la pluma arriba, asi que se rechaza ensenando el nombre bueno.
#
# Las dos son firmware CORRECTO. Un pack que solo supiera contar ramas las acusaria, y
# entonces sus acusaciones no valdrian nada -es la misma exigencia que app_03 se pone
# con SET_TIEMPOS: quien sabe acusar tiene que saber reconocer-. Asi que la clasificacion
# tiene TRES estados, no dos:
#
#     IMPLEMENTA           la rama existe y puede contestar $ACK
#     RECHAZA CON MOTIVO   la rama existe, solo contesta $ERR, y el $ERR NOMBRA la orden
#     AUSENTE              no hay rama: cae en el $ERR,CMD:DESCONOCIDO generico
#
# Y ademas se EXIGE la asimetria: de toda orden que sea de una sola punta y tenga rama en
# la otra, se comprueba que esa rama es un rechazo que dice el motivo. Si alguien la
# dejara caer al DESCONOCIDO generico, quien la mande no se entera de como se llama ahora
# -que es justo lo que N-83 corrigio-.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna, y la tentacion era ponerle
# SFTY-27 -"el Esclavo pide, no ordena"- porque SOLICITAR_PASO sale en sus cuentas. No la
# ejerce: comprueba que la app sepa a quien mandar esa orden, no que el Esclavo se
# abstenga de encender nada. Eso lo mide esclavo_06_no_abre_paso. Una regla que aparece
# cubierta por una prueba que no la ejerce es peor que una fila vacia.

import re

NOMBRE = "app_08_enrutado_por_punta"
DESCRIPCION = "el reparto de comandos entre Maestro y Esclavo que la app lleva escrito sale de las ramas del C++"

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
BT_MAESTRO = ("Maestro", "src", "bluetooth.cpp")
BT_ESCLAVO = ("Esclavo", "src", "bluetooth.cpp")

# Los nombres de las dos listas del .js. No son datos: son las DOS PUERTAS por las que
# la app decide, y si alguna se renombra este pack tiene que ABORTAR en vez de aprobar
# comparando contra un conjunto vacio. Es lo que le paso al banco entero en N-75, cuando
# un pack buscaba en app.js una rama que se habia mudado a js/.
LISTAS = {"Maestro": "SOLO_MAESTRO", "Esclavo": "SOLO_ESCLAVO"}

# La cadena de strcmp/strncmp ES el contrato: no hay tabla ni enum que leer. Traido
# literal de app_03_sin_ok_mudo, que ya lo tenia probado.
_RAMA = re.compile(r'\bstrn?cmp\s*\(\s*(?:accion|cmd)\s*,\s*"([^"]+)"')

# El prefijo del PIN no es una rama de comando: es el filtro de autenticacion.
_PIN = re.compile(r"^CMD:PIN:\d+:$")

_LISTA_JS = r"const\s+%s\s*=\s*\[(.*?)\]\s*;"


def _bloque(texto, i):
    """El interior del bloque que abre en texto[i] == '{'. None si no cierra.

    Traido literal de app_03_sin_ok_mudo. Reescribirlo para renombrar una variable es
    como se cuelan los errores en un cambio que no debe cambiar comportamiento."""
    if i < 0 or i >= len(texto) or texto[i] != "{":
        return None
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "{":
            prof += 1
        elif texto[j] == "}":
            prof -= 1
            if prof == 0:
                return texto[i + 1:j]
    return None


def _despachador(fw, partes):
    """El cuerpo de procesarComando() de esa punta."""
    codigo = fw.codigo(*partes)
    m = re.search(r"\bprocesarComando\s*\([^)]*\)\s*\{", codigo)
    if not m:
        return None
    return _bloque(codigo, m.end() - 1)


def _acciones(cuerpo):
    """{accion: bloque} de esa punta, con la accion normalizada como la nombra la app.

    Tres normalizaciones, y las tres salen de como esta escrito el propio despachador:

      - Se tira el prefijo "CMD:" de las ramas que comparan contra `cmd` entero -las
        ordenes sin PIN-, porque en la cadena de `accion` la misma orden aparece sin el.
        Sin esto FORZAR_ROJO contaria como dos ordenes distintas segun por que puerta
        entre, y una de las dos se quedaria sin vigilar.
      - Se tira el ':' final de los strncmp -"SET_TIEMPOS:" es la raiz de la orden, y
        los tres numeros que van detras no son parte de su nombre-.
      - Se saltan las ramas que solo montan la autenticacion: las que asignan `accion`
        y la del PIN. No son comandos; son el filtro de la puerta.

    Cuando la misma orden tiene DOS ramas -con PIN y sin PIN- se juntan sus bloques: lo
    que decide si esa punta la implementa es si ALGUNA de las dos puede contestar $ACK."""
    fuera = {}
    for m in _RAMA.finditer(cuerpo):
        etiqueta = m.group(1)
        i = cuerpo.find("{", m.end())
        bloque = _bloque(cuerpo, i)
        if bloque is None:
            continue
        if _PIN.match(etiqueta) or re.search(r"\baccion\s*=", bloque):
            continue
        accion = etiqueta[4:] if etiqueta.startswith("CMD:") else etiqueta
        accion = accion.rstrip(":")
        if not accion:
            continue
        fuera[accion] = fuera.get(accion, "") + bloque
    return fuera


def _implementa(bloque):
    """La rama puede contestar que si. Un $ACK es la unica promesa que hay en el cable."""
    return '"$ACK' in bloque


def _rechaza_con_motivo(accion, bloque):
    """La rama existe SOLO para decir que no, y dice por que.

    Se exige que el $ERR NOMBRE LA ORDEN -$ERR,CMD:TEST_LEDS,...- y no el DESCONOCIDO
    generico. Es la diferencia entre "esta punta no lo hace, y esto es lo que tienes que
    hacer" y "no se de que me hablas": quien manda una orden retirada tiene una app o un
    manual anteriores al cambio, y lo que necesita no es enterarse de que no existe."""
    if _implementa(bloque):
        return False
    raiz = accion.split(":")[0]
    return bool(re.search(r'"\$ERR,CMD:(?:%s|%s)\b' % (re.escape(accion), re.escape(raiz)),
                          bloque))


def _listas_de_la_app(js):
    """{punta: [entradas]} leidas de las dos listas escritas a mano en app.js."""
    fuera = {}
    for punta, nombre in LISTAS.items():
        m = re.search(_LISTA_JS % nombre, js, re.S)
        if m is None:
            return None, nombre
        fuera[punta] = re.findall(r"'([^']+)'", m.group(1))
    return fuera, None


def _cubre(entrada, accion):
    """La entrada de la lista gobierna esa accion.

    La app enruta por la RAIZ de la orden -corta por el primer ':' antes de consultar la
    lista-, asi que 'SET_MODO' gobierna las siete SET_MODO:X. Se admite tambien la
    entrada escrita entera, que es como esta MANUAL:CAMBIAR_TURNO."""
    return accion == entrada or accion.startswith(entrada + ":")


def correr(b, fw):
    b.titulo("El reparto de comandos por punta, recalculado desde los dos despachadores")

    # ---- 1. Las ramas de cada despachador, leidas del C++ ----
    ramas = {}
    for punta, partes in (("Maestro", BT_MAESTRO), ("Esclavo", BT_ESCLAVO)):
        cuerpo = _despachador(fw, partes)
        if cuerpo is None:
            raise fw.Abortado(
                "%s: no se hallo procesarComando() en bluetooth.cpp. Es el unico sitio "
                "donde vive el reparto de comandos: sin el, este pack compararia las "
                "listas de la app contra nada y saldria verde" % punta)
        ramas[punta] = _acciones(cuerpo)
        if len(ramas[punta]) < 3:
            raise fw.Abortado(
                "%s: el despachador solo dio %d rama(s) de comando. Las dos puntas "
                "atienden varias desde V9.0: o el despachador dejo de ser una cadena de "
                "comparaciones o el buscador se quedo atras, y medir tres ramas "
                "aprobaria cualquier reparto" % (punta, len(ramas[punta])))

    implementa = {p: {a for a, blq in ramas[p].items() if _implementa(blq)} for p in ramas}
    if not implementa["Maestro"] or not implementa["Esclavo"]:
        raise fw.Abortado(
            "una de las dos puntas no implementa NI UNA orden segun el censo (Maestro "
            "%d, Esclavo %d). Las dos contestan $ACK desde V8.4: fallo el detector de "
            "$ACK, no el firmware"
            % (len(implementa["Maestro"]), len(implementa["Esclavo"])))

    solo = {p: sorted(implementa[p] - implementa[q])
            for p, q in (("Maestro", "Esclavo"), ("Esclavo", "Maestro"))}
    ambas = sorted(implementa["Maestro"] & implementa["Esclavo"])

    b.verificar(
        True,
        "reparto derivado del C++: solo Maestro %s | solo Esclavo %s | las dos %s"
        % (solo["Maestro"], solo["Esclavo"], ambas),
        "no deberia llegarse aqui")

    # ---- 2. Las dos listas de la app ----
    js = fw.texto_repo(*APP_JS)
    listas, falta = _listas_de_la_app(js)
    if listas is None:
        raise fw.Abortado(
            "no se hallo en app.js la lista %s. El reparto por punta vive SOLO ahi: si "
            "se renombro o se mudo de fichero, este pack no puede medir nada -y es "
            "exactamente como el banco entero se quedo en ABORTADO en N-75, con la app "
            "entrando sin vigilancia detras-" % falta)

    # ---- 3. Toda orden de UNA sola punta esta gobernada por la lista de esa punta ----
    for punta in ("Maestro", "Esclavo"):
        sin_gobernar = [a for a in solo[punta]
                        if not any(_cubre(e, a) for e in listas[punta])]
        b.verificar(
            not sin_gobernar,
            "%s: las %d ordenes que solo el %s implementa estan gobernadas por %s"
            % (punta, len(solo[punta]), punta, LISTAS[punta]),
            "%s: %s las implementa SOLO el %s y %s no las nombra. Pulsadas contra la "
            "otra punta salen al cable, el despachador cae al else y contesta "
            "$ERR,CMD:DESCONOCIDO: el boton parece roto y el error no dice nada"
            % (punta, sin_gobernar, punta, LISTAS[punta]))

    # ---- 4. Y ninguna lista secuestra una orden que las DOS puntas atienden ----
    for punta in ("Maestro", "Esclavo"):
        secuestradas = [a for a in ambas if any(_cubre(e, a) for e in listas[punta])]
        b.verificar(
            not secuestradas,
            "%s: %s no reclama ninguna de las %d ordenes que atienden las dos puntas (%s)"
            % (punta, LISTAS[punta], len(ambas), ambas),
            "%s: %s reclama %s, que las DOS puntas implementan. La app desviaria al "
            "tecnico a la otra punta -o le negaria la orden- por una restriccion que el "
            "firmware no tiene" % (punta, LISTAS[punta], secuestradas))

    # ---- 5. La direccion contraria: ninguna entrada de la lista sobra ----
    #
    # Sin esto la lista se llena de nombres obsoletos y deja de significar nada, que es
    # el mismo trinquete que costura_10 le pone a las funciones huerfanas: sobra tanto
    # una que falta como una que ya no corresponde a nada.
    for punta, otra in (("Maestro", "Esclavo"), ("Esclavo", "Maestro")):
        huecas = [e for e in listas[punta]
                  if not any(_cubre(e, a) for a in solo[punta])]
        b.verificar(
            not huecas,
            "%s: las %d entradas de %s corresponden a ordenes que hoy son de esa punta"
            % (punta, len(listas[punta]), LISTAS[punta]),
            "%s: %s nombra %s y el %s ya no implementa ninguna orden que empiece asi. O "
            "la orden se retiro, o cambio de punta y la lista se quedo atras: la app "
            "sigue desviando al tecnico por un reparto que el firmware ya no tiene"
            % (punta, LISTAS[punta], huecas, punta))

        invasoras = [e for e in listas[punta]
                     if any(_cubre(e, a) for a in implementa[otra])]
        b.verificar(
            not invasoras,
            "%s: ninguna entrada de %s reclama una orden que el %s implementa"
            % (punta, LISTAS[punta], otra),
            "%s: %s nombra %s y el %s TAMBIEN las implementa. La lista dice 'esto es de "
            "una sola punta' de algo que atienden las dos, asi que la app manda al "
            "tecnico a cambiar de poste para nada" % (punta, LISTAS[punta], invasoras, otra))

    # ---- 6. La asimetria deliberada se EXIGE, no se supone ----
    #
    # Esta es la comprobacion que separa "esta punta no lo implementa" de "esta punta lo
    # rechaza a proposito y lo dice". Sin ella el pack sabria acusar y no reconocer, y
    # entonces no valdria ninguna de sus acusaciones.
    for punta, otra in (("Maestro", "Esclavo"), ("Esclavo", "Maestro")):
        for accion in solo[punta]:
            if accion not in ramas[otra]:
                continue
            b.verificar(
                _rechaza_con_motivo(accion, ramas[otra][accion]),
                "%s / %s: el %s la rechaza NOMBRANDOLA, no la deja caer al DESCONOCIDO "
                "generico -asimetria deliberada, no un hueco-" % (otra, accion, otra),
                "%s / %s: el %s tiene rama para esa orden y ni la implementa ni dice por "
                "que la niega. Quien la mande tiene una app o un manual anteriores al "
                "cambio, y lo que necesita no es enterarse de que no existe sino de como "
                "se llama ahora (N-83)" % (otra, accion, otra))

    # ---- 7. Controles negativos ----
    #
    # Se ejercen sobre despachadores sinteticos con la MISMA forma que el real. Si el
    # detector aprobara el defectuoso, todos los OK de arriba serian decoracion.
    movido = ('if (strcmp(accion, "TEST_LEDS") == 0) { '
              'enviarTramaConCrc("$ACK,CMD:TEST_LEDS,RESULT:OK"); } '
              'else if (strcmp(accion, "OTRA") == 0) { x(); }')
    rechazo = ('if (strcmp(accion, "TEST_LEDS") == 0) { '
               'enviarTramaConCrc("$ERR,CMD:TEST_LEDS,DESC:NO_EN_SERVICIO"); }')
    mudo = ('if (strcmp(accion, "TEST_LEDS") == 0) { '
            'enviarTramaConCrc("$ERR,CMD:DESCONOCIDO,DESC:NO"); }')

    b.control_negativo(
        _implementa(_acciones(movido)["TEST_LEDS"]),
        "una orden que cambia de punta -pasa a contestar $ACK donde antes solo negaba- "
        "se detecta como implementada, que es lo que dejaria la lista de la app atras")
    b.control_negativo(
        _rechaza_con_motivo("TEST_LEDS", _acciones(rechazo)["TEST_LEDS"]),
        "y un rechazo que NOMBRA la orden NO se marca: el detector distingue la "
        "asimetria deliberada de un hueco, en vez de acusar a todo el que dice que no")
    b.control_negativo(
        not _rechaza_con_motivo("TEST_LEDS", _acciones(mudo)["TEST_LEDS"]),
        "mientras que un rechazo que cae al $ERR,CMD:DESCONOCIDO generico SI se marca")
    b.control_negativo(
        not any(_cubre(e, "SET_RTC") for e in ("SET_MODO", "SET_TIEMPOS")),
        "la relacion de cobertura no da por gobernada una orden que solo comparte el "
        "prefijo de tres letras con una entrada de la lista")
    b.control_negativo(
        _cubre("SET_MODO", "SET_MODO:DEGRADADO") and not _cubre("SET_MODO", "SET_MODOS"),
        "y SI cubre la raiz con su argumento -SET_MODO gobierna SET_MODO:DEGRADADO- sin "
        "tragarse un nombre que solo empieza igual")
