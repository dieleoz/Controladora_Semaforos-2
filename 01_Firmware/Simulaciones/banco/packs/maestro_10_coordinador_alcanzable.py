# ===== banco/packs/maestro_10_coordinador_alcanzable.py =====
#
# EL COORDINADOR NO BASTA CON LLAMARLO: HAY QUE PODER LLEGAR A LA LLAMADA.
#
# LA PROPIEDAD QUE VIGILA ESTE PACK, escrita antes que ningun sintoma:
#
#   Todo modo que main.cpp EXCLUYE del refresco de fondo
#   -coordinador_actualizar_background()- tiene que poder llegar a
#   coordinador_actualizar() desde CUALQUIER estado alcanzable de su maquina; o,
#   si no llama al coordinador en absoluto, ese silencio tiene que ser una
#   propiedad MEDIBLE del modo y no un descuido.
#
# POR QUE EXISTE, con la fecha y el dano medido (04/09/2026).
#
# SET_MODO:AUTO entraba en modoAutomatico_setup(), que dejaba `fase = CONFIG_ROJO`
# -un cuestionario de tres pantallas de la epoca del LCD-. La UNICA salida de ese
# estado era botonAceptar(), y botonAceptar() devuelve `false` SIEMPRE desde el
# 31/08, cuando BOTON3 y BOTON4 pasaron a ser entradas de camara. El modo entraba y
# no salia. Como coordinador_actualizar() vivia DENTRO del `case CORRIENDO`, y
# main.cpp excluye MODO_AUTOMATICO del refresco de fondo con el comentario "ya se
# llama en modo_automatico.cpp", el Maestro se quedaba MUDO en la radio: ni un
# PING. El Esclavo, sin oir nada, se iba a ambar por orfandad haciendo lo correcto,
# y desde fuera parecia una averia de comunicaciones.
#
# LA EXCLUSION Y LA LLAMADA ESTABAN DE ACUERDO SOBRE EL PAPEL Y EN DESACUERDO EN
# EJECUCION. Es S2.ter del CLAUDE.md en estado puro: algo DECLARADO -"la llamada
# existe"- frente a ese algo EJERCIDO -"la llamada se alcanza"-. Un grep de
# "existe la llamada?" contesta que si y no mide nada. Por eso este pack no busca
# la llamada: calcula si se llega a ella.
#
# POR QUE NO SE VIGILA EL SINTOMA. `CONFIG_ROJO` desaparece del firmware el mismo
# dia en que se escribe esto. Un pack que sepa decir "existe CONFIG_ROJO" es basura
# manana. Aqui no aparece el nombre de ningun estado, ningun modo ni ningun boton:
# la lista de exclusiones se lee de main.cpp, los modos se resuelven por el
# despachador, los ficheros por censo del directorio y las funciones de valor
# constante por censo del fuente. Si el lector no encuentra lo que busca, ABORTA
# -que no dice nada del firmware- en vez de suponer.
#
# POR QUE NO LLEVA ETIQUETA "EJERCE SFTY-x". La consecuencia del defecto la cobra
# SFTY-6 -el ambar por orfandad del Esclavo-, pero este pack NO ejerce SFTY-6: no
# cuenta el silencio ni mueve un temporizador; lee el Maestro y decide si puede
# hablar. Etiquetarlo pondria una fila cubierta en la tabla de trazabilidad por una
# prueba que no la ejerce, y eso es peor que una fila vacia porque la vacia no
# miente (CLAUDE.md, Convenciones).
#
# ---------------------------------------------------------------------------------
# LO QUE ESTE PACK NO PUEDE COMPROBAR - y va escrito aqui para que su verde no se
# lea como permiso. Tambien sale impreso en cada corrida, por si alguien lee el acta
# y no el fichero.
#
#   1. NO EJECUTA NADA. Es analisis del texto del C++, no del binario ni de la
#      tarjeta. Que la llamada sea alcanzable no dice que el coordinador emita, ni
#      que la radio module, ni que el Esclavo lo oiga. Eso es banco.
#   2. NO EVALUA EL TIEMPO. Un estado que llama al coordinador cada 30 s pasa igual
#      que uno que lo llama cada vuelta, y el primero deja al Esclavo huerfano. La
#      desigualdad contra SFTY6_SILENCIO_MS no se mide aqui.
#   3. NO ENTIENDE `else`, ni el operador ternario, ni `goto`, ni las maquinas de
#      estado que no sean un `switch` sobre una variable `static` de un `enum`
#      declarado en el mismo fichero. Cuando no entiende una guarda la da por
#      POSIBLE: la direccion que no acusa al firmware de lo que no se ha medido.
#      Esa eleccion tiene un coste declarado: un estado trampa cuya salida imposible
#      este escrita de una forma que el evaluador no sepa leer pasaria como bueno.
#   4. NO VIGILA A LOS MODOS NO EXCLUIDOS. A esos les refresca el coordinador
#      main.cpp en cada vuelta, y ese es justo el motivo de que no esten aqui.
#   5. NO DECIDE SI UN SILENCIO ES BUENO. Mide que un modo silencioso mueva sus
#      propias luces; que ademas SEA correcto que esa punta calle -y que el Esclavo
#      se vaya a ambar- es una decision vial, no una propiedad del fuente. Si esta
#      lista de modos silenciosos crece, sale impresa en cada acta para que se vea.
#   6. NO MIRA AL ESCLAVO. La exclusion vive en el main.cpp del Maestro.
# ---------------------------------------------------------------------------------

import re

NOMBRE = "maestro_10_coordinador_alcanzable"
DESCRIPCION = "en todo modo excluido del refresco de fondo, el coordinador se ALCANZA"


# ---------------------------------------------------------------------------------
# LECTURA DEL C++ - un analizador de bloques, no una lista de patrones sueltos.
# ---------------------------------------------------------------------------------

_LITERAL = re.compile(r'"(?:[^"\\\n]|\\.)*"' r"|'(?:[^'\\\n]|\\.)*'")


def _sin_literales(t):
    """Vacia el CONTENIDO de cadenas y caracteres conservando la longitud.

    Todo lo que hay debajo se apoya en contar llaves y parentesis, y una llave
    dentro de una cadena descuadra la cuenta entera. Se conserva la longitud a
    proposito: asi los desplazamientos siguen valiendo sobre el mismo texto y no
    hay que mantener dos copias en paralelo -que es como se acaba midiendo una
    posicion de un texto en el otro-."""
    return _LITERAL.sub(lambda m: m.group(0)[0] + "X" * (len(m.group(0)) - 2) + m.group(0)[-1], t)


def _cierra(t, i, ab, ce):
    """Indice del delimitador que cierra el que abre en t[i]. None si no cuadra."""
    if i >= len(t) or t[i] != ab:
        return None
    n = 0
    for j in range(i, len(t)):
        if t[j] == ab:
            n += 1
        elif t[j] == ce:
            n -= 1
            if n == 0:
                return j
    return None


def _prof(t, pos):
    """Profundidad de llaves en pos. Solo vale sobre texto ya sin literales."""
    return t.count("{", 0, pos) - t.count("}", 0, pos)


def _cuerpo(cod, nombre):
    """Cuerpo (sin las llaves) de la DEFINICION de `nombre`. None si no esta.

    Se distingue definicion de llamada por lo que hay tras el parentesis: una '{'.
    Sin esa distincion, `modoAutomatico_loop();` en el despachador de main.cpp
    valdria como definicion y el analisis se haria sobre el fichero equivocado."""
    for m in re.finditer(r"\b%s\s*\(" % re.escape(nombre), cod):
        j = _cierra(cod, m.end() - 1, "(", ")")
        if j is None:
            continue
        k = j + 1
        while k < len(cod) and cod[k] in " \t\r\n":
            k += 1
        if k < len(cod) and cod[k] == "{":
            f = _cierra(cod, k, "{", "}")
            if f is not None:
                return cod[k + 1:f]
    return None


def _guardas(b):
    """[(ini, fin, condicion)] de cada `if (...)` del bloque, con su region.

    Se admiten las dos formas -con llaves y de una sola sentencia- porque el
    firmware usa las dos. El `else` NO se modela: una asignacion dentro de un else
    se queda fuera de toda region y se toma por incondicional, que es la direccion
    que no acusa."""
    res = []
    for m in re.finditer(r"\bif\s*\(", b):
        j = _cierra(b, m.end() - 1, "(", ")")
        if j is None:
            continue
        cond = b[m.end():j]
        k = j + 1
        while k < len(b) and b[k] in " \t\r\n":
            k += 1
        if k < len(b) and b[k] == "{":
            f = _cierra(b, k, "{", "}")
            if f is not None:
                res.append((k, f, cond))
        else:
            f = b.find(";", k)
            if f >= 0:
                res.append((k, f, cond))
    return res


def _guarda_de(guardas, pos):
    """La guarda MAS INTERNA que encierra pos, o None si no hay ninguna."""
    dentro = [g for g in guardas if g[0] < pos < g[1]]
    if not dentro:
        return None
    return min(dentro, key=lambda g: g[1] - g[0])[2]


def _evaluar(cond, constantes):
    """True / False / None sobre una condicion hecha SOLO de llamadas constantes.

    None significa "no se sabe", y no es lo mismo que False: una transicion que no
    se sabe si existe se da por POSIBLE. Preferir None a adivinar es lo que impide
    que este pack acuse a un firmware por una guarda que no supo leer -S4: un 'no
    aparece' no es un hallazgo hasta haber descartado al buscador-."""
    e = cond
    faltan = []

    def _sust(m):
        n = m.group(1)
        if n in constantes:
            return "True" if constantes[n] else "False"
        faltan.append(n)
        return "?"

    e = re.sub(r"\b(\w+)\s*\(\s*\)", _sust, e)
    if faltan:
        return None
    e = e.replace("&&", " and ").replace("||", " or ").replace("!", " not ")
    e = e.replace("true", "True").replace("false", "False")
    if not re.fullmatch(r"[\s()]*(?:(?:not\s+)*(?:True|False)[\s()]*(?:(?:and|or)[\s()]*)?)+", e):
        return None
    try:
        return bool(eval(e, {"__builtins__": {}}, {}))  # noqa: S307
    except Exception:  # noqa: BLE001
        return None


def _censo_constantes(fw, punta, carpeta):
    """Funciones sin argumentos cuyo cuerpo es UN `return` de literal.

    Censa el DIRECTORIO, no una lista escrita a mano: el dia que aparezca la
    tercera funcion de estas -o desaparezcan las dos de hoy- el censo cambia solo.
    Es la generalizacion de botonAceptar()/botonCancelar(), que devuelven `false`
    desde que J16 p10 y p12 son camaras: una funcion de valor constante no puede
    ser la unica salida de un estado."""
    consts = {}
    for n in fw.fuentes_de(punta, carpeta):
        c = _sin_literales(fw.codigo(punta, carpeta, n))
        pat = r"\b(\w+)\s*\(\s*(?:void\s*)?\)\s*\{\s*return\s+(true|false)\s*;\s*\}"
        for m in re.finditer(pat, c):
            consts[m.group(1)] = (m.group(2) == "true")
    return consts


# ---------------------------------------------------------------------------------
# EL ANALISIS DE UN MODO.
#
# Toma el TEXTO del fichero, no una ruta, para que los controles negativos puedan
# atacarlo con fuentes sinteticos. Un analizador que solo sabe leer del disco no se
# puede ver fallar sin tocar el firmware, y el firmware no se toca (S8.bis).
# ---------------------------------------------------------------------------------

LLAMADA = r"\bcoordinador_actualizar\s*\(\s*\)"


def _maquina(cod, cuerpo_loop):
    """(variable, valores, cuerpo_del_switch) de la maquina de estados. None si no hay.

    Se exige que la variable sea `static` de un `enum` declarado en el MISMO
    fichero: sin eso, cualquier `switch` sobre un parametro pasaria por maquina de
    estados y el calculo de alcanzabilidad mediria otra cosa."""
    for m in re.finditer(r"switch\s*\(\s*(\w+)\s*\)\s*\{", cuerpo_loop):
        var = m.group(1)
        dm = re.search(r"\bstatic\s+(\w+)\s+%s\b" % re.escape(var), cod)
        if not dm:
            continue
        em = re.search(r"\benum\s+%s\s*\{([^}]*)\}" % re.escape(dm.group(1)), cod)
        if not em:
            continue
        valores = [v.strip().split("=")[0].strip()
                   for v in em.group(1).split(",") if v.strip()]
        k = m.end() - 1
        f = _cierra(cuerpo_loop, k, "{", "}")
        if f is None:
            continue
        return var, valores, cuerpo_loop[k + 1:f]
    return None


def _casos(sw):
    """{etiqueta: bloque} del switch. Corta en la siguiente etiqueta de nivel 0."""
    marcas = [(m.start(), m.group(1), m.end())
              for m in re.finditer(r"\bcase\s+(\w+)\s*:", sw) if _prof(sw, m.start()) == 0]
    out = {}
    for i, (ini, etq, fin) in enumerate(marcas):
        tope = marcas[i + 1][0] if i + 1 < len(marcas) else len(sw)
        out[etq] = sw[fin:tope]
    return out


def analizar(cod, nom_loop, nom_setup, constantes):
    """Diagnostico de un modo. Devuelve un dict; nunca lanza por el contenido.

    clase:
      'silencioso'    el fichero no llama al coordinador NI UNA VEZ.
      'incondicional' hay una llamada al nivel 0 del cuerpo del loop.
      'maquina'       las llamadas son condicionales y hay maquina de estados: se
                      calcula la co-alcanzabilidad.
      'indecidible'   las llamadas son condicionales y NO se encontro maquina. El
                      pack no puede decidir, y decirlo es su trabajo.
    """
    cod = _sin_literales(cod)
    d = {"loop": nom_loop, "setup": nom_setup, "llamadas": len(re.findall(LLAMADA, cod))}

    cuerpo_loop = _cuerpo(cod, nom_loop)
    if cuerpo_loop is None:
        d["clase"] = "sin_loop"
        return d
    d["luces_nivel0"] = any(_prof(cuerpo_loop, m.start()) == 0
                            for m in re.finditer(r"\bsemaforo_actualizar\s*\(\s*\)", cuerpo_loop))

    if d["llamadas"] == 0:
        d["clase"] = "silencioso"
        return d

    if any(_prof(cuerpo_loop, m.start()) == 0 for m in re.finditer(LLAMADA, cuerpo_loop)):
        d["clase"] = "incondicional"
        # Una salida temprana por encima puede saltarse la llamada sin que las
        # llaves lo delaten. No se cuenta como fallo -es una guarda legitima- pero
        # se dice, porque el pack no evalua su condicion.
        pos = min(m.start() for m in re.finditer(LLAMADA, cuerpo_loop)
                  if _prof(cuerpo_loop, m.start()) == 0)
        d["return_antes"] = bool(re.search(r"\breturn\s*;", cuerpo_loop[:pos]))
        return d

    maq = _maquina(cod, cuerpo_loop)
    if maq is None:
        d["clase"] = "indecidible"
        return d

    var, valores, sw = maq
    casos = _casos(sw)
    d.update({"var": var, "valores": valores, "casos": sorted(casos)})

    # Estado de entrada: TODA asignacion del setup cuenta como inicial posible. Si
    # el setup tuviera dos ramas, quedarse con una seria elegir el escenario comodo.
    cuerpo_setup = _cuerpo(cod, nom_setup) if nom_setup else None
    if cuerpo_setup is None:
        d["clase"] = "indecidible"
        d["motivo"] = "no se encontro el setup del modo"
        return d
    iniciales = [m.group(1) for m in
                 re.finditer(r"\b%s\s*=\s*(\w+)\s*;" % re.escape(var), cuerpo_setup)
                 if m.group(1) in valores]
    if not iniciales:
        d["clase"] = "indecidible"
        d["motivo"] = "el setup no fija el estado de entrada de la maquina"
        return d
    d["iniciales"] = iniciales

    # Transiciones. Las que estan fuera del switch valen desde CUALQUIER estado.
    trans = {e: set() for e in valores}
    g_loop = _guardas(cuerpo_loop)
    ini_sw = cuerpo_loop.find(sw)
    for m in re.finditer(r"\b%s\s*=\s*(\w+)\s*;" % re.escape(var), cuerpo_loop):
        if m.group(1) not in valores:
            continue
        if ini_sw >= 0 and ini_sw <= m.start() < ini_sw + len(sw):
            continue
        if _evaluar(_guarda_de(g_loop, m.start()) or "true", constantes) is False:
            continue
        for e in valores:
            trans[e].add(m.group(1))
    for etq, bloque in casos.items():
        g = _guardas(bloque)
        for m in re.finditer(r"\b%s\s*=\s*(\w+)\s*;" % re.escape(var), bloque):
            if m.group(1) not in valores or m.group(1) == etq:
                continue
            if _evaluar(_guarda_de(g, m.start()) or "true", constantes) is False:
                continue          # guarda DEMOSTRADA falsa: esa salida no existe
            trans.setdefault(etq, set()).add(m.group(1))

    objetivo = {e for e, b in casos.items() if re.search(LLAMADA, b)}

    def _cierre(semilla, aristas):
        vistos, pila = set(semilla), list(semilla)
        while pila:
            x = pila.pop()
            for y in aristas.get(x, ()):
                if y not in vistos:
                    vistos.add(y)
                    pila.append(y)
        return vistos

    inversas = {e: set() for e in valores}
    for a, bs in trans.items():
        for bb in bs:
            inversas.setdefault(bb, set()).add(a)

    alcanzables = _cierre(iniciales, trans)
    co_alcanzan = _cierre(objetivo, inversas)

    d.update({
        "clase": "maquina",
        "objetivo": sorted(objetivo),
        "alcanzables": sorted(alcanzables),
        "mudos": sorted(alcanzables - co_alcanzan),
        "trampas": sorted(e for e in alcanzables
                          if not trans.get(e) and e not in objetivo),
        "transiciones": {a: sorted(b) for a, b in trans.items() if b},
    })
    return d


# ---------------------------------------------------------------------------------
# FUENTES SINTETICOS PARA LOS CONTROLES NEGATIVOS.
#
# No son adorno: son la unica forma de ver fallar a este pack sin tocar un .cpp que
# otro agente esta editando. Los dos primeros son el MISMO modo con y sin el defecto
# del 04/09 - si el analizador no distingue esos dos, su PASS no vale nada.
# ---------------------------------------------------------------------------------

_SINT_MALO = """
enum FaseX { CONFIG_A, CORRIENDO };
static FaseX fase;
void x_setup() { fase = CONFIG_A; }
void x_loop() {
  switch (fase) {
    case CONFIG_A: {
      if (botonAceptar()) { fase = CORRIENDO; }
      break;
    }
    case CORRIENDO: {
      coordinador_actualizar();
      break;
    }
  }
}
"""

_SINT_BUENO = _SINT_MALO.replace("fase = CONFIG_A;", "fase = CORRIENDO;")

# La misma maquina, pero la salida la abre un boton VIVO. Sin este tercer caso el
# control negativo mediria una tapia: un analizador que declarase mudo TODO estado
# sin la llamada pasaria los dos de arriba igual de bien que el correcto (S8.sexies).
_SINT_SALIDA_VIVA = _SINT_MALO.replace("botonAceptar()", "botonArriba()")


# ---------------------------------------------------------------------------------

def correr(b, fw):
    verificar, propiedad, control_negativo = b.verificar, b.propiedad, b.control_negativo
    titulo, reportar = b.titulo, b.reportar

    main = _sin_literales(fw.codigo("Maestro", "src", "main.cpp"))

    # --- 10.1 EL CENSO DE EXCLUSIONES ---------------------------------------------
    # De donde sale la lista: del propio main.cpp. Una lista escrita a mano aqui
    # envejeceria sin avisar el dia que alguien anada o quite un modo, y este pack
    # seguiria en verde vigilando un firmware que ya no existe.
    titulo("10.1 que modos excluye main.cpp del refresco de fondo")

    llam_fondo = list(re.finditer(r"\bcoordinador_actualizar_background\s*\(\s*\)", main))
    if not llam_fondo:
        # Descartado el buscador: el simbolo esta declarado en coordinador.h. Que no
        # aparezca en main.cpp no es un fallo del lector, es otro firmware.
        decl = re.search(r"coordinador_actualizar_background",
                         fw.texto("Maestro", "include", "coordinador.h"))
        raise fw.Abortado(
            "main.cpp no llama a coordinador_actualizar_background() (declarada en "
            "coordinador.h: %s). Sin esa llamada no hay 'lista de exclusiones' que "
            "censar y este pack no puede decidir nada del firmware."
            % ("si" if decl else "NO"))

    m_guarda = re.search(
        r"\bif\s*\(([^{}]*?)\)\s*\{[^{}]*?coordinador_actualizar_background\s*\(\s*\)\s*;", main, re.S)
    if not m_guarda:
        raise fw.Abortado(
            "coordinador_actualizar_background() aparece %d vez/veces en main.cpp pero "
            "NO dentro de un `if (...) { ... }` que se pueda leer. Sin poder leer la "
            "guarda no se sabe que modos quedan excluidos, y suponerlo seria inventar "
            "la propiedad que este pack dice medir." % len(llam_fondo))

    cond = m_guarda.group(1)
    excluidos = re.findall(r"!=\s*(MODO_\w+|MENU)\b", cond)
    incluidos = re.findall(r"==\s*(MODO_\w+|MENU)\b", cond)

    # La guarda tiene que ser una lista de EXCLUSION. Si alguien la volviera del
    # reves -`if (modo == MODO_MANUAL)`- este lector leeria "cero excluidos" y el
    # pack pasaria en verde sobre un firmware donde TODOS los demas modos estan
    # mudos. El lector se comprueba antes de fiarse de lo que lee (S4).
    verificar(bool(excluidos) and not incluidos,
              "La guarda del refresco de fondo es una lista de EXCLUSION (%d modo(s) con "
              "'!='): %s. El censo se puede leer." % (len(excluidos), ", ".join(excluidos)),
              "La guarda del refresco de fondo NO es una lista de exclusion legible: "
              "excluye %r e incluye %r sobre la condicion %r. Este pack leeria la lista "
              "al reves y aprobaria un firmware con modos mudos"
              % (excluidos, incluidos, cond.strip()))

    control_negativo(
        bool(re.findall(r"==\s*(MODO_\w+)\b", "if (modo == MODO_MANUAL) { x(); }"))
        and not re.findall(r"!=\s*(MODO_\w+)\b", "if (modo == MODO_MANUAL) { x(); }"),
        "el lector de la guarda distingue una lista de inclusion de una de exclusion")

    # --- 10.2 CADA MODO EXCLUIDO SE RESUELVE A UN FICHERO ---------------------------
    # Sin lista escrita a mano tampoco aqui: el despachador de main.cpp dice que
    # funcion corre cada modo, y el censo del directorio dice en que .cpp vive.
    titulo("10.2 cada modo excluido se resuelve a un fichero por censo")

    disp_loop = dict(re.findall(r"case\s+(MODO_\w+|MENU)\s*:\s*(\w+_loop)\s*\(", main))
    disp_setup = dict(re.findall(r"case\s+(MODO_\w+|MENU)\s*:\s*(\w+_setup)\s*\(", main))
    fuentes = fw.fuentes_de("Maestro", "src")

    resueltos, sin_resolver = {}, []
    for modo in excluidos:
        nl, ns = disp_loop.get(modo), disp_setup.get(modo)
        duenos = [n for n in fuentes
                  if nl and _cuerpo(_sin_literales(fw.codigo("Maestro", "src", n)), nl) is not None]
        if not nl or len(duenos) != 1:
            sin_resolver.append("%s -> loop %r en %d fichero(s)" % (modo, nl, len(duenos)))
        else:
            resueltos[modo] = (duenos[0], nl, ns)

    verificar(not sin_resolver,
              "Los %d modos excluidos se resuelven a un unico .cpp cada uno, censando "
              "Maestro/src: %s" % (len(excluidos),
                                   ", ".join("%s=%s" % (m, v[0]) for m, v in sorted(resueltos.items()))),
              "Hay modos excluidos que no se resuelven a un fichero: %s. Un modo "
              "excluido del refresco de fondo que nadie sabe donde vive no se puede "
              "vigilar" % "; ".join(sin_resolver))

    # --- 10.3 EL CENSO DE FUNCIONES DE VALOR CONSTANTE -----------------------------
    titulo("10.3 funciones de valor constante: una salida que no abre")

    constantes = _censo_constantes(fw, "Maestro", "src")

    # El censo tiene que saber distinguir un cuerpo constante de uno real. Si no,
    # daria la lista vacia, ninguna guarda se podria demostrar falsa, y el calculo
    # de estados trampa se apagaria sin que nada fallara -N-89: un instrumento en
    # verde midiendo nada-.
    _pat = r"\b(\w+)\s*\(\s*(?:void\s*)?\)\s*\{\s*return\s+(true|false)\s*;\s*\}"
    control_negativo(
        bool(re.search(_pat, "bool f() { return false; }"))
        and not re.search(_pat, "bool g() { return consumir(0); }"),
        "el censo de funciones constantes reconoce `return false;` y NO marca un cuerpo real")

    # --- 10.4 LA PROPIEDAD ----------------------------------------------------------
    titulo("10.4 desde todo estado alcanzable se puede LLEGAR al coordinador")

    silenciosos, indecidibles, diag = [], [], {}
    for modo, (fich, nl, ns) in sorted(resueltos.items()):
        d = analizar(fw.codigo("Maestro", "src", fich), nl, ns, constantes)
        d["fichero"] = fich
        diag[modo] = d

        if d["clase"] == "sin_loop":
            indecidibles.append("%s: no se encontro el cuerpo de %s() en %s" % (modo, nl, fich))
            continue

        if d["clase"] == "silencioso":
            silenciosos.append(modo)
            # El motivo de la excepcion se MIDE, no se redacta (S2.ter). main.cpp
            # justifica el silencio de estos modos escribiendo que "llaman a
            # semaforo_actualizar() por su cuenta". Eso es una afirmacion sobre el
            # codigo: o se comprueba, o no vale como excepcion.
            verificar(d.get("luces_nivel0"),
                      "%s esta excluido y NO llama al coordinador -silencio deliberado-, y "
                      "mueve sus propias luces: %s llama a semaforo_actualizar() sin "
                      "condicion. El motivo de la excepcion esta medido, no escrito."
                      % (modo, nl),
                      "%s esta excluido del refresco de fondo, no llama al coordinador Y "
                      "TAMPOCO refresca sus luces sin condicion en %s: en ese modo el "
                      "Maestro calla en la radio y ademas depende de que otro le mueva el "
                      "cabezal" % (modo, nl))
            continue

        if d["clase"] == "indecidible":
            indecidibles.append("%s (%s): %s" % (modo, fich,
                                                 d.get("motivo", "llamadas condicionales sin maquina de estados legible")))
            continue

        if d["clase"] == "incondicional":
            propiedad(True,
                      "%s: coordinador_actualizar() se llama al nivel 0 de %s(), asi que se "
                      "ejecuta en toda vuelta del modo. No hace falta calcular nada."
                      % (modo, nl),
                      "")   # inalcanzable: la condicion es literalmente True
            if d.get("return_antes"):
                reportar("%s tiene un `return` por encima de la llamada" % modo,
                         ["La llamada esta al nivel 0 de %s(), pero hay un `return;` antes." % nl,
                          "Este pack NO evalua esa guarda: si algun dia se vuelve siempre",
                          "cierta, la llamada deja de ejecutarse y aqui seguiria en verde."])
            continue

        # clase == 'maquina': aqui esta el trabajo.
        ok = not d["mudos"]
        detalle_trampas = (" Estados sin ninguna salida posible: %s." % ", ".join(d["trampas"])) \
            if d["trampas"] else ""
        propiedad(
            ok,
            "%s: desde %s la maquina `%s` alcanza %s, y TODOS pueden llegar a un estado que "
            "llama a coordinador_actualizar() (%s)."
            % (modo, "/".join(d["iniciales"]), d["var"], ", ".join(d["alcanzables"]),
               ", ".join(d["objetivo"]) or "ninguno"),
            "%s ESTA MUDO EN LA RADIO. Entra por %s y los estados %s son alcanzables sin "
            "poder llegar nunca a coordinador_actualizar() (que solo vive en %s).%s "
            "main.cpp excluye este modo del refresco de fondo, asi que ahi no hay quien "
            "supla la llamada: el Maestro queda vivo pero sin hablar, y el Esclavo se ira a "
            "ambar por orfandad."
            % (modo, "/".join(d["iniciales"]), ", ".join(d["mudos"]),
               ", ".join(d["objetivo"]) or "ningun estado", detalle_trampas))

    # Un 'indecidible' no se apunta para luego: es una puerta abierta (S3.quater). Se
    # aborta el pack entero -que no dice nada del firmware- en vez de contarlo como
    # una comprobacion que paso.
    if indecidibles:
        raise fw.Abortado(
            "hay modos excluidos cuyas llamadas al coordinador son TODAS condicionales y "
            "cuya maquina de estados no se pudo leer: %s. Este pack no puede decidir si la "
            "llamada se alcanza, y aprobarlo seria justo el defecto que vigila"
            % "; ".join(indecidibles))

    # --- 10.5 CONTROLES NEGATIVOS DEL ANALIZADOR -----------------------------------
    titulo("10.5 el analizador sabe fallar")

    # Los sinteticos se alimentan del censo REAL para que no midan un mundo aparte,
    # pero con sus dos nombres fijados a mano: el defecto que se inyecta necesita una
    # salida demostrablemente muerta y la del tercer caso una demostrablemente viva.
    # Si manana el firmware volviera vivo a botonAceptar() o matara a botonArriba(),
    # el control negativo seguiria midiendo lo que dice medir en vez de apagarse.
    cn = dict(constantes)
    cn["botonAceptar"] = False
    cn.pop("botonArriba", None)

    d_malo = analizar(_SINT_MALO, "x_loop", "x_setup", cn)
    d_bueno = analizar(_SINT_BUENO, "x_loop", "x_setup", cn)
    d_vivo = analizar(_SINT_SALIDA_VIVA, "x_loop", "x_setup", cn)

    control_negativo(d_malo["clase"] == "maquina" and d_malo["mudos"] == ["CONFIG_A"],
                     "sobre un modo sintetico que entra en un estado cuya UNICA salida es una "
                     "funcion de valor constante, el analizador lo marca mudo (%s)"
                     % (d_malo.get("mudos"),))

    control_negativo(d_bueno["clase"] == "maquina" and not d_bueno["mudos"],
                     "sobre EL MISMO modo sintetico con el estado de entrada arreglado, el "
                     "analizador NO acusa: sabe distinguir el arreglo del defecto")

    # Sin este tercero, un analizador que declarase mudo todo estado sin la llamada
    # pasaria los dos de arriba. Aqui la salida existe de verdad y no debe acusarse.
    control_negativo(d_vivo["clase"] == "maquina" and not d_vivo["mudos"],
                     "con la MISMA forma pero una salida que si puede abrirse, el analizador no "
                     "acusa: mide la salida, no la ausencia de la llamada")

    # --- 10.6 LO QUE QUEDA DICHO ----------------------------------------------------
    titulo("10.6 el censo publicado y lo que este pack NO mide")

    silencio_ms = fw.constante(("Maestro", "include", "protocolo.h"),
                               r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
                               "el silencio que manda al Esclavo a ambar por orfandad")

    lineas = ["Guarda leida en main.cpp: %s" % cond.strip()]
    for modo in sorted(diag):
        d = diag[modo]
        lineas.append("  %-18s %-22s clase=%-13s llamadas=%d %s"
                      % (modo, d["fichero"], d["clase"], d["llamadas"],
                         ("estados %s desde %s" % (",".join(d.get("alcanzables", [])),
                                                   "/".join(d.get("iniciales", [])))
                          if d["clase"] == "maquina" else "")))
    lineas.append("Funciones de valor constante censadas en Maestro/src: %s"
                  % (", ".join("%s()->%s" % (k, str(v).lower())
                               for k, v in sorted(constantes.items())) or "ninguna"))
    if silenciosos:
        lineas.append("MODOS QUE CALLAN EN LA RADIO A PROPOSITO: %s. Mientras uno de esos"
                      % ", ".join(silenciosos))
        lineas.append("esta puesto, el Esclavo deja de oir al Maestro y a los %d ms se va a"
                      % silencio_ms)
        lineas.append("ambar por orfandad. Que eso sea correcto es una decision vial, no una")
        lineas.append("propiedad del fuente: si esta lista crece, se decide, no se hereda.")
    reportar("censo de la exclusion del refresco de fondo", lineas)

    # main.cpp justifica la exclusion diciendo que los modos excluidos llaman a
    # semaforo_actualizar() "por su cuenta, que es lo que aqui se perderia". Se mide
    # tambien la otra mitad de esa frase, porque una frase al lado de un instrumento
    # verde es lo que sostuvo tres de los cinco defectos del 3-4/09.
    cuerpo_main = _cuerpo(main, "loop")
    luces_en_main = bool(cuerpo_main) and any(
        _prof(cuerpo_main, m.start()) == 0
        for m in re.finditer(r"\bsemaforo_actualizar\s*\(\s*\)", cuerpo_main))
    if luces_en_main and silenciosos:
        reportar("la frase que justifica la exclusion describe un firmware anterior",
                 ["main.cpp llama a semaforo_actualizar() SIN CONDICION en su propio loop,",
                  "asi que 'los dos modos las llaman por su cuenta, que es lo que aqui se",
                  "perderia' ya no es cierto en su segunda mitad: no se perderia nada.",
                  "La exclusion sigue siendo correcta por el OTRO motivo -callar en la",
                  "radio-, que es el que este pack mide. No es un defecto; es una frase",
                  "con autoridad de dato que envejecio, y por eso queda escrita aqui."])

    reportar("lo que este pack NO puede comprobar",
             ["1. No ejecuta nada: lee el C++. Que la llamada sea alcanzable no dice que",
              "   el coordinador emita, ni que el Esclavo lo oiga. Eso es banco.",
              "2. No mide TIEMPO. Un estado que llame al coordinador cada 30 s pasa igual",
              "   que uno que lo llama cada vuelta, y el primero deja huerfano al Esclavo.",
              "3. No entiende `else`, ternarios, `goto` ni maquinas que no sean un switch",
              "   sobre una `static` de un `enum` del mismo fichero. Lo que no entiende lo",
              "   da por POSIBLE, asi que una salida imposible escrita de otra forma",
              "   pasaria por buena.",
              "4. No vigila a los modos NO excluidos: a esos les refresca main.cpp.",
              "5. No decide si callar es correcto en un modo. Mide que el modo mueva sus",
              "   luces; que la punta deba callar es una decision vial.",
              "6. No mira al Esclavo."])
