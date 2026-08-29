# ===== banco/packs/app_02_modos_simetricos.py =====
#
# LO QUE EL EQUIPO SABE NOMBRAR, EL EQUIPO TIENE QUE SABER OBEDECER.
#
# LA PROPIEDAD, EN UNA LINEA: todo literal de modo que obtenerNombreModo() puede
# EMITIR en la trama $STATUS tiene una rama SET_MODO:<literal> que lo ACEPTA en el
# despachador, y al reves.
#
# POR QUE EXISTE ESTE PACK.
#
# app_01_comandos vigila la costura entre la app y el firmware -que lo que el .js
# manda alguien lo atienda-. Esa costura no ve NADA de lo de aqui, porque este
# desajuste vive ENTERO dentro del mismo .cpp: la telemetria y el despachador estan
# a ochenta lineas uno del otro y no comparten ni una tabla. Son dos listas escritas
# a mano en dos sitios, y nadie las cruzaba.
#
# El desajuste medido el 28/08, antes de tocar nada:
#
#   obtenerNombreModo() sabe DECIR ......  MENU MANUAL AUTO INTELIGENTE ALCANCE
#                                          HORA DEGRADADO AMBAR
#   el despachador sabe OBEDECER .......   AUTO MANUAL AMBAR
#
# O sea: el telefono puede LEER "MODO:DEGRADADO" en la pantalla y no tiene ninguna
# forma de pedirlo NI DE SALIR DE EL. Lo mismo con MENU, ALCANCE e INTELIGENTE. Un
# tecnico ve el estado del equipo y no puede cambiarlo: la telemetria funciona como
# un espejo, y el mando no llega.
#
# Y LO QUE ESTE PACK IMPIDE MAS ALLA DE HOY: el proximo modo que alguien anada
# nacera otra vez legible y no ordenable -porque anadir un `case` al switch de la
# telemetria es lo primero que se hace y anadir la rama del despachador es lo que se
# olvida-, y nadie lo notara hasta que un tecnico lo pida en campo.
#
# COMO SE MIDE: LEYENDO EL C++, NUNCA UNA LISTA ESCRITA A MANO.
#
# Los dos lados salen del fuente en cada corrida -los `case X: return "Y";` del
# switch, y los strcmp(accion, "SET_MODO:Y") del despachador-. Si alguno de los dos
# censos sale vacio esto ABORTA: comparar contra un conjunto vacio aprueba cualquier
# cosa, y un ABORTADO al menos grita.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna. Ejerce una propiedad de
# simetria del interfaz de mando, no una regla de seguridad; ponerle SFTY-18 porque
# HORA aparece en la lista seria hacer figurar como cubierta una regla que aqui no
# se ejerce, y una fila que miente es peor que una vacia.

import re

NOMBRE = "app_02_modos_simetricos"
DESCRIPCION = "todo modo que la telemetria sabe nombrar, el despachador sabe obedecerlo"

PUNTAS = ("Maestro", "Esclavo")

# EXCEPCIONES DECLARADAS POR NOMBRE, CON SU MOTIVO. No se esconden en un filtro
# generico: quien lea el pack tiene que poder discutir cada una.
#
# La regla para admitir una: el modo existe como ESTADO INTERNO pero pedirlo desde
# el telefono no significa nada o deja al equipo en una situacion que nadie atiende.
SIN_SET_MODO_A_PROPOSITO = {
    "HORA":
        "MODO_HORA es la pantalla AJUSTAR HORA (SFTY-18), que se edita DIGITO A "
        "DIGITO con los botones fisicos o el mando de reles y solo escribe al RTC "
        "al confirmar. Mandarla por Bluetooth dejaria al equipo plantado en una "
        "pantalla de edicion que nadie esta operando en el poste, y sin ciclo. La "
        "via remota para poner el reloj ya existe y hace el trabajo completo: es "
        "SET_RTC, que el propio SFTY-18 nombra como sustituto de esa pantalla.",
}

# El rotulo del `default:` del switch no es un modo: es lo que la telemetria dice
# cuando le llega un valor del enum que no sabe nombrar. Se excluye ESTRUCTURALMENTE
# -abajo solo se cuentan los `case`- y ademas se comprueba que nadie lo acepte como
# orden, que es la unica forma de que la exclusion tenga dientes.


def _bloque(texto, i):
    """El interior del bloque que abre en texto[i] == '{'. None si no cierra."""
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


def _formato_status(codigo):
    """La cadena de formato de la trama $STATUS que esa punta emite."""
    m = re.search(r'"(\$STATUS[^"]*)"', codigo)
    return m.group(1) if m else None


def _diccionario(codigo):
    """Los `case X: return "Y";` de obtenerNombreModo(), y su rotulo `default:`.

    Devuelve (None, None) si la funcion no esta: quien llama decide si eso es una
    punta sin diccionario -legitimo- o un buscador roto -ABORTADO-."""
    m = re.search(r"\bobtenerNombreModo\s*\([^)]*\)\s*\{", codigo)
    if not m:
        return None, None
    cuerpo = _bloque(codigo, m.end() - 1)
    if cuerpo is None:
        return None, None
    casos = dict(re.findall(r'case\s+(\w+)\s*:\s*return\s+"([^"]+)"\s*;', cuerpo))
    porDefecto = re.search(r'default\s*:\s*return\s+"([^"]+)"\s*;', cuerpo)
    return casos, (porDefecto.group(1) if porDefecto else None)


def _acepta_set_modo(codigo):
    """Los literales que el despachador acepta como SET_MODO:<literal>."""
    return set(re.findall(r'strcmp\s*\(\s*accion\s*,\s*"SET_MODO:([^"]+)"', codigo))


# DONDE VIVE EL ENUM, CON EL ROL ESCRITO Y POR PUNTA.
#
# Hasta el 28/08 esto era fw.codigo(punta, "include", "menu.h"): el enum ModoSistema
# vivia dentro de la cabecera de la pantalla. Al sacarlo a modos.h -el modo lo
# consultan main.cpp, mando.cpp y bluetooth.cpp, que no dibujan nada- la ruta se
# reapunta EN EL MISMO COMMIT que el traslado. Si no, este pack no encontraria el enum
# y saldria ABORTADO, que no es PASS: dejaria entrar sin mirar toda la simetria de
# modos que vigila.
#
# Y SE ESCRIBE COMO TUPLA COMPLETA, NO COMO ("include", "modos.h"), QUE NO ES
# COSMETICA. La guarda de rutas de compuerta.py exige EN LAS DOS PUNTAS cualquier
# pareja carpeta/fichero que aparezca sin rol -asi es como vigila los ficheros de
# costura, que si estan duplicados-. El Esclavo NO tiene ModoSistema: su $STATUS
# publica MODO:SUBORDINADO fijo, censado 0 veces en Esclavo/src y Esclavo/include
# contra 48 en el Maestro. Sin el rol delante, la guarda pediria un
# Esclavo\include\modos.h que no existe ni debe existir, y abortaria.
RUTA_ENUM = {
    "Maestro": ("Maestro", "include", "modos.h"),
}


def _enum_modos(fw, punta):
    """Los valores del enum ModoSistema de esa punta, leidos de su cabecera."""
    ruta = RUTA_ENUM.get(punta)
    if ruta is None:
        return set()
    m = re.search(r"enum\s+ModoSistema\s*\{([^}]*)\}", fw.codigo(*ruta))
    if not m:
        return set()
    return set(re.findall(r"\b([A-Z][A-Z0-9_]*)\b", m.group(1)))


def correr(b, fw):
    b.titulo("Simetria de modos: lo que se sabe decir se tiene que saber obedecer")

    # ---- Censo previo: que punta publica un modo VARIABLE y cual uno fijo ----
    #
    # No se da por sentado que el diccionario viva en el Maestro. Se pregunta al
    # C++: una punta cuya trama $STATUS lleva "MODO:%s" necesita diccionario; una
    # que lleva "MODO:SUBORDINADO" no tiene modos que ofrecer y no se le exige uno.
    conDiccionario = []
    for p in PUNTAS:
        codigo = fw.codigo(p, "src", "bluetooth.cpp")
        fmt = _formato_status(codigo)
        if fmt is None:
            raise fw.Abortado(
                "%s: no se hallo la cadena de formato de $STATUS en bluetooth.cpp. "
                "Sin ella este pack no sabe si esa punta publica un modo variable o "
                "uno fijo, y decidirlo a ojo seria inventarse el contrato" % p)
        casos, _ = _diccionario(codigo)
        variable = "MODO:%s" in fmt
        if variable:
            conDiccionario.append(p)
        b.verificar(
            variable == (casos is not None),
            "%s: la trama $STATUS y el diccionario de modos concuerdan (%s)"
            % (p, "MODO variable con obtenerNombreModo()" if variable
               else "MODO fijo, sin diccionario que mantener"),
            "%s: la trama $STATUS %s pero obtenerNombreModo() %s. Una punta que "
            "publica MODO:%%s sin diccionario emitiria basura de memoria; una que "
            "publica un modo FIJO y mantiene un diccionario tiene una lista que "
            "nadie usa y que nadie va a actualizar"
            % (p, "publica MODO:%s" if variable else "publica un MODO fijo",
               "no existe" if casos is None else "si existe"))

    if not conDiccionario:
        raise fw.Abortado(
            "ninguna punta publica MODO:%s en su $STATUS, asi que no hay diccionario "
            "de modos que cruzar. O el firmware cambio de trama o el buscador de este "
            "pack se quedo atras; en cualquiera de los dos casos aqui no se ha medido "
            "nada del reparto de modos")

    # ---- El resto se mide sobre la punta que si tiene modos que ofrecer ----
    for p in conDiccionario:
        codigo = fw.codigo(p, "src", "bluetooth.cpp")
        casos, rotuloDefecto = _diccionario(codigo)

        if not casos:
            raise fw.Abortado(
                "%s: obtenerNombreModo() no dio ni un `case X: return \"Y\";`. El "
                "switch es la unica lista de modos que existe y compararla vacia "
                "aprobaria cualquier despachador" % p)
        if rotuloDefecto is None:
            raise fw.Abortado(
                "%s: obtenerNombreModo() no tiene rama `default:`. Este pack cuenta "
                "con ella para saber que rotulo NO es un modo; sin identificarla "
                "estaria a punto de exigir una rama SET_MODO para el comodin" % p)

        # 1. El diccionario nombra TODOS los valores del enum.
        #
        # Un valor del enum que se cayera al `default:` saldria por telemetria como
        # el rotulo comodin: el tecnico veria un equipo "DESCONOCIDO" que en realidad
        # esta en un modo perfectamente definido.
        enum = _enum_modos(fw, p)
        if not enum:
            raise fw.Abortado(
                "%s: no se pudo leer su enum ModoSistema (RUTA_ENUM lo situa en %s). "
                "O esa punta gano un diccionario de modos sin declarar aqui donde "
                "vive su enum, o la cabecera cambio de sitio. Sin el no hay "
                "contra que contrastar los `case` del switch"
                % (p, RUTA_ENUM.get(p, "NINGUNA RUTA DECLARADA")))
        sinNombrar = sorted(enum - set(casos))
        b.verificar(
            not sinNombrar,
            "%s: obtenerNombreModo() nombra los %d valores de ModoSistema: %s"
            % (p, len(enum), sorted(casos.values())),
            "%s: el enum ModoSistema tiene %s y el switch no los nombra, asi que "
            "caen al `default:` y salen por telemetria como '%s'. El equipo estaria "
            "en un modo definido y el tecnico leeria el comodin"
            % (p, sinNombrar, rotuloDefecto))

        emite = set(casos.values())
        acepta = _acepta_set_modo(codigo)

        # La forma por prefijo no la sabe leer este censo. Si aparece, el conjunto
        # `acepta` deja de ser el contrato completo y el pack acusaria de faltar
        # ramas que si existen. Eso es ABORTADO -hay que arreglar el pack-, nunca
        # PASS ni FALLA: ninguno de los dos diria la verdad.
        if re.search(r'strncmp\s*\(\s*accion\s*,\s*"SET_MODO:"', codigo):
            raise fw.Abortado(
                "%s: el despachador compara SET_MODO por PREFIJO (strncmp). Este "
                "pack censa ramas exactas strcmp(accion,\"SET_MODO:X\") y con la "
                "forma por prefijo su censo se queda corto: acusaria al firmware de "
                "no aceptar modos que si acepta. Se arregla el pack" % p)
        if not acepta:
            raise fw.Abortado(
                "%s: el despachador no tiene ni una rama strcmp(accion,\"SET_MODO:"
                "X\"). O cambio de forma o el buscador esta roto; comparar contra "
                "vacio marcaria como huerfano TODO el diccionario y el informe "
                "sonaria a catastrofe sin haber medido nada" % p)

        # 2. TODO lo que se sabe decir se sabe obedecer.
        exigibles = emite - set(SIN_SET_MODO_A_PROPOSITO)
        noOrdenables = sorted(exigibles - acepta)
        b.verificar(
            not noOrdenables,
            "%s: los %d modos exigibles tienen rama SET_MODO (%s); exceptuados por "
            "escrito: %s"
            % (p, len(exigibles), sorted(exigibles),
               sorted(SIN_SET_MODO_A_PROPOSITO) or "ninguno"),
            "%s: la telemetria sabe EMITIR %s y el despachador NO los acepta como "
            "SET_MODO. El telefono lee el estado del equipo y no puede ni pedirlo ni "
            "salir de el -con DEGRADADO eso significa un equipo que se declara en "
            "operacion por reloj y sin mando remoto para sacarlo-. El despachador "
            "solo obedece %s" % (p, noOrdenables, sorted(acepta)))

        # 3. Y al reves: nada se acepta que la telemetria no sepa confirmar.
        #
        # Esta direccion caza el mando mudo: la orden entra, el modo cambia, y la
        # linea de estado sigue diciendo otra cosa. El tecnico repite la orden
        # creyendo que no le hacen caso.
        sinConfirmar = sorted(acepta - emite)
        b.verificar(
            not sinConfirmar,
            "%s: todo SET_MODO aceptado tiene su literal en la telemetria" % p,
            "%s: el despachador acepta SET_MODO:%s y obtenerNombreModo() no emite "
            "ese literal. La orden entraria y la linea $STATUS nunca lo confirmaria: "
            "el tecnico no tiene forma de saber si le hicieron caso"
            % (p, sinConfirmar))

        # 4. El rotulo del `default:` no es un modo, y nadie puede pedirlo.
        b.verificar(
            rotuloDefecto not in acepta,
            "%s: '%s' es el comodin del `default:` y NO se acepta como orden"
            % (p, rotuloDefecto),
            "%s: el despachador acepta SET_MODO:%s, que es el rotulo del `default:` "
            "-lo que la telemetria dice cuando NO sabe donde esta el equipo-. Pedir "
            "'no se' como modo no significa nada y deja al equipo donde estaba, "
            "contestando que si" % (p, rotuloDefecto))

        for modo, motivo in sorted(SIN_SET_MODO_A_PROPOSITO.items()):
            if modo in emite:
                b.reportar(
                    "%s: %s se emite y NO se acepta, a proposito" % (p, modo),
                    [motivo])

    # ---- 5. Controles negativos ----
    #
    # Los dos lectores se ejercen contra un texto que TRAE el defecto, para que su
    # PASS de arriba signifique algo. Sin esto, el dia que el switch cambiara de
    # forma los dos conjuntos saldrian vacios y todo compararia nada contra nada.
    casosFalsos, defectoFalso = _diccionario(
        'static const char* obtenerNombreModo(ModoSistema m) {'
        '  switch (m) {'
        '    case MODO_INVENTADO: return "INVENTADO";'
        '    default: return "DESCONOCIDO";'
        '  }'
        '}')
    b.control_negativo(
        casosFalsos == {"MODO_INVENTADO": "INVENTADO"} and defectoFalso == "DESCONOCIDO",
        "el lector del switch saca 'INVENTADO' de un `case` y distingue el `default:` "
        "del comodin (leyo %s / %s)" % (casosFalsos, defectoFalso))

    b.control_negativo(
        _acepta_set_modo('strcmp(accion, "SET_MODO:INVENTADO") == 0') == {"INVENTADO"}
        and _acepta_set_modo('enviarTramaConCrc("$ACK,CMD:SET_MODO:AUTO,RESULT:OK");') == set(),
        "el lector del despachador encuentra la rama SET_MODO y NO se traga el "
        "literal del $ACK, que menciona el mismo texto sin ser una rama")
