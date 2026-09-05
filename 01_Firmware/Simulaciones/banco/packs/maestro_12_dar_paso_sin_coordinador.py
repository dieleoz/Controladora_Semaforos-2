# ===== banco/packs/maestro_12_dar_paso_sin_coordinador.py =====
#
# NO SE ACEPTA UN DAR PASO EN UN MODO DONDE NADIE MUEVE EL COORDINADOR.
#
# LA PROPIEDAD, y es una relacion entre DOS ficheros que hoy nadie ata:
#
#   Todo modo al que main.cpp NIEGA el refresco de fondo y que ademas NO llama al
#   coordinador desde su propio loop() tiene que ser un modo en el que el despachador
#   de Bluetooth RECHACE la peticion de cambio de turno. Ni uno menos.
#
# POR QUE EXISTE, con la cinta delante (05/09/2026).
#
# El equipo en MODO:AMBAR y tres MANUAL:CAMBIAR_TURNO seguidos en 40 s, los tres
# contestados "$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE". El operario leia
# "el cruce esta cambiando de fase, repita al terminar" y el cambio NO TERMINABA NUNCA.
#
# La causa no se parecia al sintoma. main.cpp EXCLUYE a MODO_AMBAR y a MODO_DEGRADADO del
# refresco de fondo -en esos dos el Maestro calla en la radio a proposito- y sus loop() no
# llaman al coordinador: alli la maquina esta CONGELADA. La PRIMERA pulsacion si entraba
# -estadoC valia C_IDLE- y dejaba el coordinador en un estado de transicion que YA NO
# AVANZA. Desde ahi, todas las demas caian en el "if (estadoC != C_IDLE) return;" y
# contestaban lo mismo hasta que alguien cambiara de modo.
#
# O sea: el equipo dijo que SI a una orden que no iba a ejecutar, Y ADEMAS SE QUEDO PEOR
# QUE ANTES DE PEDIRLA. Es la barrera de salidas (CLAUDE.md 6) en su forma mas cara, y el
# mensaje era encima mentiroso: no habia ninguna transicion en curso.
#
# POR QUE NO SE VIGILA EL SINTOMA. Aqui no se escribe a mano el nombre de ningun modo: la
# lista de excluidos se LEE de la condicion de main.cpp, el fichero de cada modo se
# resuelve por el switch, y de la guarda del despachador se leen los modos que nombra. Si
# manana un tercer modo deja de llamar al coordinador, este pack lo exige en la guarda sin
# que nadie se acuerde de anadirlo. Si el lector no encuentra lo que busca, ABORTA.
#
# POR QUE NO LLEVA ETIQUETA "EJERCE SFTY-x". Toca el silencio deliberado de SFTY-21 pero
# no lo ejerce: no comprueba que el Maestro calle, comprueba que no prometa lo que no
# puede hacer mientras calla. Etiquetar una regla con una prueba que no la ejerce es peor
# que dejar la fila vacia.

import re

NOMBRE = "maestro_12_dar_paso_sin_coordinador"
DESCRIPCION = "DAR PASO se rechaza en los modos donde el coordinador esta congelado"

MAIN = ("Maestro", "src", "main.cpp")
BT = ("Maestro", "src", "bluetooth.cpp")

# Lo unico nombrado a mano, y son hechos del reparto de ficheros, no del diseno que se
# mide: como se llama el refresco de fondo y como se llama el paso del coordinador.
REFRESCO = "coordinador_actualizar_background"
PASO = "coordinador_actualizar"


def _sin_comentarios(cod):
    """Sin esto el pack se acusa a si mismo: los comentarios de este repositorio CITAN
    los nombres de modo que explican, y cualquier busqueda por texto los encuentra."""
    cod = re.sub(r"/\*.*?\*/", " ", cod, flags=re.S)
    return re.sub(r"//[^\n]*", "", cod)


def _medir(fw):
    main = _sin_comentarios(fw.codigo(*MAIN))
    bt = _sin_comentarios(fw.codigo(*BT))

    # --- Los modos que main.cpp NIEGA al refresco de fondo -------------------
    m = re.search(r"if\s*\(([^)]*)\)\s*\{\s*%s\s*\(" % re.escape(REFRESCO), main)
    if not m:
        raise fw.Abortado(
            "no se hallo en main.cpp el 'if (...) { %s(); }'. De esa condicion sale la "
            "lista de modos sin refresco, que es la mitad izquierda de lo que este pack "
            "compara; escribirla a mano seria el valor por defecto que este banco no "
            "admite" % REFRESCO)
    excluidos = set(re.findall(r"modo\s*!=\s*(MODO_\w+|MENU)", m.group(1)))
    if len(excluidos) < 2:
        raise fw.Abortado(
            "la condicion del refresco de fondo solo niega %d modo(s) (%s). Con menos de "
            "dos no hay una lista que comparar: el pack estaria aprobando el vacio"
            % (len(excluidos), sorted(excluidos)))

    # --- Que loop() corre cada modo, leido del switch de main.cpp ------------
    loops = dict(re.findall(r"case\s+(MODO_\w+|MENU)\s*:\s*(\w+)\s*\(\s*\)\s*;", main))
    faltan = [e for e in excluidos if e not in loops]
    if faltan:
        raise fw.Abortado(
            "el switch de main.cpp no dice que loop() corren %s. Sin saberlo no se puede "
            "comprobar si ese modo mueve el coordinador por su cuenta, y suponerlo en "
            "cualquiera de los dos sentidos falsea el resultado" % sorted(faltan))

    # --- De esos, cuales SI llaman al coordinador por su cuenta --------------
    # El fichero de cada modo se busca por su loop, censando el directorio: una tabla
    # modo->fichero escrita aqui envejeceria en el primer renombrado.
    def mueve(nombre_loop):
        for f in fw.fuentes_de("Maestro", "src"):
            cod = _sin_comentarios(fw.codigo("Maestro", "src", f))
            d = re.search(r"\b%s\s*\([^;{]*\)\s*\{" % re.escape(nombre_loop), cod)
            if not d:
                continue
            i = cod.index("{", d.end() - 1)
            prof, fin = 0, len(cod)
            for j in range(i, len(cod)):
                if cod[j] == "{": prof += 1
                elif cod[j] == "}":
                    prof -= 1
                    if prof == 0:
                        fin = j; break
            return re.search(r"\b%s\s*\(" % re.escape(PASO), cod[i:fin]) is not None
        return None

    congelados, sin_hallar = set(), []
    for e in sorted(excluidos):
        r = mueve(loops[e])
        if r is None:
            sin_hallar.append(loops[e])
        elif not r:
            congelados.add(e)
    if sin_hallar:
        raise fw.Abortado(
            "no se hallo la definicion de %s en Maestro/src. Es el loop de un modo sin "
            "refresco de fondo: sin leerlo no se sabe si ese modo mueve el coordinador"
            % sorted(sin_hallar))

    # --- Los modos que el despachador rechaza --------------------------------
    g = re.search(r"bool\s+modoMueveElCoordinador\s*\([^)]*\)\s*\{(.*?)\n\}", bt, re.S)
    if not g:
        raise fw.Abortado(
            "no se hallo modoMueveElCoordinador() en bluetooth.cpp. Es la guarda que "
            "este pack existe para vigilar: sin ella no hay nada que medir, y aprobar "
            "aqui seria dar por buena una barrera que no se ha leido")
    rechazados = set(re.findall(r"m\s*!=\s*(MODO_\w+|MENU)", g.group(1)))

    return {"excluidos": excluidos, "congelados": congelados,
            "rechazados": rechazados, "loops": loops}


def correr(b, fw):
    b.titulo("DAR PASO no se acepta donde el coordinador esta congelado")
    d = _medir(fw)
    cong, rech = d["congelados"], d["rechazados"]

    b.verificar(
        True,
        "censo leido del C++: main.cpp niega el refresco de fondo a %s; de esos, %s no "
        "llaman al coordinador desde su loop"
        % (", ".join(sorted(d["excluidos"])), ", ".join(sorted(cong)) or "ninguno"),
        "-")

    # ---- 1. Todo modo congelado esta rechazado -----------------------------
    b.verificar(
        cong <= rech,
        "los %d modo(s) donde el coordinador esta congelado (%s) los rechaza la guarda "
        "de DAR PASO: la orden no se acepta para luego no poder cumplirla"
        % (len(cong), ", ".join(sorted(cong)) or "ninguno"),
        "%s deja(n) el coordinador congelado -sin refresco de fondo y sin llamarlo desde "
        "su loop- y la guarda de DAR PASO NO lo(s) rechaza. La primera pulsacion entra, "
        "deja la maquina en una transicion que nadie va a terminar, y a partir de ahi el "
        "operario recibe 'el cruce esta cambiando de fase, repita' PARA SIEMPRE"
        % ", ".join(sorted(cong - rech)))

    # ---- 2. Y no se rechaza de mas (la mitad positiva) ---------------------
    #
    # Sin esta linea la 1 seria una TAPIA: una guarda que rechazara DAR PASO en TODOS los
    # modos la pasaria igual de bien que la correcta, y habria dejado el boton muerto en
    # Manual, que es justo el modo para el que existe. Es 8.sexies.
    b.verificar(
        rech <= cong,
        "y no rechaza de mas: la guarda no nombra ningun modo que si mueva el coordinador",
        "la guarda rechaza DAR PASO en %s, que SI mueve(n) el coordinador. Rechazar de "
        "mas deja el boton muerto en un modo donde funcionaba, y eso no lo dice ningun "
        "sintoma: el operario pulsa y le contestan que no" % ", ".join(sorted(rech - cong)))

    # ---- Controles negativos: los dos defectos, inyectados -----------------
    class _Parche:
        def __init__(self, fw, viejo, nuevo):
            self.fw, self.viejo, self.nuevo = fw, viejo, nuevo
            self.Abortado = fw.Abortado
        def codigo(self, *p):
            c = self.fw.codigo(*p)
            if p == BT:
                if c.count(self.viejo) != 1:
                    raise self.fw.Abortado(
                        "el parche de un control negativo no encontro su ancla una sola "
                        "vez (%d): %r. Sin inyectar el defecto, el control aprobaria el "
                        "firmware sano diciendo que sabe detectar"
                        % (c.count(self.viejo), self.viejo))
                return c.replace(self.viejo, self.nuevo)
            return c
        def __getattr__(self, n):
            return getattr(self.fw, n)

    if cong:
        uno = sorted(cong)[0]
        roto = _medir(_Parche(fw, "m != %s" % uno, "m != MODO_ALCANCE"))
        b.control_negativo(
            not (roto["congelados"] <= roto["rechazados"]),
            "quitado %s de la guarda -el defecto de la cinta-, la comprobacion 1 cae" % uno)

    vivos = sorted(set(d["loops"]) - cong)
    if vivos:
        roto = _medir(_Parche(fw, "return m != ", "return m != %s && m != " % vivos[0]))
        b.control_negativo(
            not (roto["rechazados"] <= roto["congelados"]),
            "anadido %s a la guarda -rechazar de mas-, la comprobacion 2 cae" % vivos[0])
