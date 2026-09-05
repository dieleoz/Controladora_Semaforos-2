# ===== banco/packs/maestro_11_manual_no_cicla.py =====
#
# EL MODO MANUAL NO PROGRAMA CAMBIOS DE LUZ. LOS PIDE UNA PERSONA O NO OCURREN.
#
# LA PROPIEDAD, escrita antes que ningun sintoma:
#
#   El modo que se opera A MANO tiene que quedar, al entrar, en el MISMO estado del
#   coordinador desde el que se acepta una peticion de cambio. Ni antes -o la primera
#   pulsacion se rechaza- ni con un cambio ya programado -o el cruce se mueve sin que
#   nadie lo pida-.
#
# POR QUE EXISTE, con la fecha y el dano medido (04-05/09/2026).
#
# modoManual_setup() llamaba a coordinador_iniciarModo(), que es LA ENTRADA DEL MODO
# AUTOMATICO: deja el coordinador en el estado de espera del despeje, o sea con un verde
# ya programado para dentro de tiempoDespejeMs. Eso producia las dos mitades del defecto
# que se reporto desde el banco, y las dos se miden en el mismo numero:
#
#   1. DAR PASO NO HACIA NADA durante esos segundos, porque coordinador_pedirCambio()
#      abre con "if (estadoC != C_IDLE) return;". El operario pulsaba y el cruce se
#      quedaba en rojo.
#   2. Y AL VENCER EL PLAZO EL CRUCE CAMBIABA SOLO, sin que nadie hubiera pulsado.
#
# Reportado asi: "el boton dar paso maestro queda en rojo, pasan 15 seg y ... pasa a
# ambar intermitente". LOS 15 SEGUNDOS SON LITERALES: tiempoDespejeMs vale 15000 ms, y
# ese ambar es la transicion rojo->AMBAR 4 s->verde que el propio Maestro arrancaba al
# vencer el plazo. El equipo estaba haciendo un ciclo que nadie le pidio.
#
# Y LA DEFINICION DEL MODO, del responsable, el 04/09: "en manual, dar paso es
# simplemente el operador le da y cambia... el operador en manual no deberia llevar un
# ciclo, sino que, como esta ahi parado viendolo, que se cambie de inmediato".
#
# EL TERCER DEFECTO, QUE NO SE HABIA REPORTADO Y SALIO AL MEDIR: el case QV_NINGUNO de
# pedirCambio() REINICIABA tRef. Con el despeje en 15 s, un operario que pulse cada 10 s
# no ve el verde NUNCA, y cada pulsacion le contesta OK. Obedecer y no avanzar es la peor
# forma de fallar, porque no deja rastro de averia.
#
# POR QUE NO SE VIGILA EL SINTOMA. Aqui no se escribe a mano el nombre de ningun estado,
# ninguna funcion de entrada ni ninguna constante: las dos entradas de todo-rojo se CENSAN
# por lo que hacen -poner la luz roja y mandar la orden de rojo por radio-, el estado que
# acepta peticiones se LEE de la guarda de pedirCambio(), y la constante del despeje se
# DEDUCE del case que la usa. Si el lector no encuentra lo que busca, ABORTA -que no dice
# nada del firmware- en vez de suponer.
#
# POR QUE NO LLEVA ETIQUETA "EJERCE SFTY-x". La comprobacion 4 toca el despeje de SFTY-4,
# pero NO lo ejerce: no mide que el todo-rojo dure lo que tiene que durar -eso es del
# arnes del automatico, que corre el ciclo de verdad-. Mide que el camino manual siga
# consultandolo. Etiquetar una regla con una prueba que no la ejerce es peor que dejar la
# fila vacia, porque la vacia no miente.

import re

NOMBRE = "maestro_11_manual_no_cicla"
DESCRIPCION = "el Modo Manual no programa cambios: la primera pulsacion entra y nada se mueve solo"

COORD = ("Maestro", "src", "coordinador.cpp")
MANUAL = ("Maestro", "src", "modo_manual.cpp")
AUTO = ("Maestro", "src", "modo_automatico.cpp")

# Lo unico que se nombra a mano, y son hechos del PROTOCOLO y del reparto de ficheros,
# no del diseno que se mide: la orden de rojo por radio y la barrera de salidas
# (CLAUDE.md 6: solo semaforo.cpp escribe pines, y su puerta de rojo es esta).
ORDEN_ROJO = "protocolo_enviarPaquete(CMD_GO_RED)"
PUERTA_ROJO = "semaforo_forzarRojo("


def _sin_comentarios(cod):
    """El fuente sin // ni /* */.

    Sin esto un pack se acusa a si mismo: los comentarios de este repositorio CITAN el
    codigo que explican -"aqui ponia coordinador_iniciarModo()"- y cualquier busqueda
    por texto los encuentra. Ya paso el 04/09."""
    cod = re.sub(r"/\*.*?\*/", " ", cod, flags=re.S)
    return re.sub(r"//[^\n]*", "", cod)


def _bloque(cod, i):
    """De la '{' en i hasta su '}'. None si no cierra."""
    prof = 0
    for j in range(i, len(cod)):
        if cod[j] == "{":
            prof += 1
        elif cod[j] == "}":
            prof -= 1
            if prof == 0:
                return cod[i:j + 1]
    return None


def _funciones(cod):
    """{nombre: cuerpo} de las definiciones de primer nivel."""
    fuera = {}
    for m in re.finditer(r"^[A-Za-z_][\w:*&<>\s]*?\b(\w+)\s*\([^;{]*\)\s*\{", cod, re.M):
        cuerpo = _bloque(cod, cod.index("{", m.end() - 1))
        if cuerpo:
            fuera[m.group(1)] = cuerpo
    return fuera


def _medir(cod_coord, cod_manual, cod_auto):
    """Todo lo que este pack sabe, resuelto a datos. Lanza ValueError si no puede leer."""
    fn = _funciones(cod_coord)

    # --- Las entradas de todo-rojo: se reconocen por lo que HACEN ---------------
    # Una entrada de todo-rojo pone la luz de esta punta en rojo Y manda la orden de
    # rojo a la otra: las dos paradas. Su nombre no se escribe aqui.
    entradas = {}
    for n, cuerpo in fn.items():
        if PUERTA_ROJO in cuerpo and ORDEN_ROJO in cuerpo:
            estados = re.findall(r"estadoC\s*=\s*(\w+)\s*;", cuerpo)
            if len(estados) == 1:
                entradas[n] = estados[0]
    if len(entradas) < 2:
        raise ValueError(
            "en coordinador.cpp hay %d entrada(s) de todo-rojo -funcion que pone el rojo "
            "local Y manda la orden de rojo, y deja UN estado-. Hacen falta al menos dos "
            "para poder distinguir la que programa un cambio de la que no; con una sola "
            "este pack no mide una eleccion, mide que existe lo unico que hay: %s"
            % (len(entradas), sorted(entradas)))
    if len(set(entradas.values())) < 2:
        raise ValueError(
            "las %d entradas de todo-rojo (%s) dejan TODAS el mismo estado. La eleccion "
            "que este pack vigila no existe en este firmware, asi que aprobarla seria "
            "aprobar el vacio" % (len(entradas), sorted(entradas)))

    # --- El estado desde el que se acepta una peticion: de la GUARDA ------------
    pedir = [n for n in fn if "pedirCambio" in n]
    if len(pedir) != 1:
        raise ValueError(
            "hay %d funcion(es) de coordinador.cpp cuyo nombre contenga 'pedirCambio' "
            "(%s). Es la puerta por la que entra DAR PASO: con ninguna no hay nada que "
            "medir y con varias no se sabe cual gobierna" % (len(pedir), sorted(pedir)))
    cuerpo_pedir = fn[pedir[0]]
    g = re.search(r"if\s*\(\s*estadoC\s*!=\s*(\w+)\s*\)\s*return", cuerpo_pedir)
    if not g:
        raise ValueError(
            "%s() ya no abre con 'if (estadoC != <estado>) return'. Ese estado es LA "
            "REFERENCIA de todo este pack -es donde tiene que quedar el modo manual para "
            "que la primera pulsacion entre-; leerlo de otro sitio seria escribirlo a "
            "mano, que es el valor por defecto que este banco no admite" % pedir[0])
    ESTADO_ACEPTA = g.group(1)

    # --- La constante del despeje: se deduce del case que la usa ---------------
    esperas = re.findall(r"millis\s*\(\s*\)\s*-\s*tRef\s*>=\s*(\w+)", cod_coord)
    if not esperas or len(set(esperas)) != 1:
        raise ValueError(
            "las esperas de despeje de coordinador.cpp nombran %s. Con ninguna no se "
            "puede comprobar que el camino manual siga consultando el despeje, y con "
            "varias distintas el despeje ya no es UN numero" % (sorted(set(esperas)) or "nada"))
    CTE_DESPEJE = esperas[0]

    # --- La rama del cruce parado dentro de pedirCambio() ----------------------
    # Se reconoce por ser el case del PRIMER enumerador de QuienVerde -el que significa
    # "nadie tiene verde"-, leido del enum y no escrito aqui.
    e = re.search(r"enum\s+QuienVerde\s*\{\s*(\w+)", cod_coord)
    if not e:
        raise ValueError(
            "no se hallo 'enum QuienVerde { ... }' en coordinador.cpp. De ahi sale cual "
            "es la rama del cruce parado; sin el habria que escribir el nombre a mano")
    QV_PARADO = e.group(1)
    mr = re.search(r"case\s+%s\s*:(.*?)break\s*;" % re.escape(QV_PARADO), cuerpo_pedir, re.S)
    if not mr:
        raise ValueError(
            "%s() no tiene un 'case %s:' con su break. Es la rama que atiende DAR PASO "
            "con el cruce parado, que es el caso del Modo Manual entero"
            % (pedir[0], QV_PARADO))
    rama_parado = mr.group(1)

    # --- Que entrada usa cada modo --------------------------------------------
    def usada_por(cod):
        return sorted(n for n in entradas if re.search(r"\b%s\s*\(" % re.escape(n), cod))

    return {
        "entradas": entradas,
        "ESTADO_ACEPTA": ESTADO_ACEPTA,
        "CTE_DESPEJE": CTE_DESPEJE,
        "manual": usada_por(cod_manual),
        "auto": usada_por(cod_auto),
        "rama_parado": rama_parado,
        "pedir": pedir[0],
    }


def _lecturas(fw, parches=()):
    """El fuente de las tres piezas, con parches EN MEMORIA (CLAUDE.md 8.bis).

    Los .cpp reales no se tocan: un arnes que edita el firmware para probarse deja el
    arbol sucio si algo revienta a mitad. Y REVIENTA SI UN PARCHE NO ENCUENTRA SU ANCLA,
    porque un ancla caducada dejaria el control negativo 'fallando bien' sin haber
    inyectado nada: seria el propio control negativo convertido en prueba muerta."""
    cods = {"coord": _sin_comentarios(fw.codigo(*COORD)),
            "manual": _sin_comentarios(fw.codigo(*MANUAL)),
            "auto": _sin_comentarios(fw.codigo(*AUTO))}
    for cual, viejo, nuevo in parches:
        if cods[cual].count(viejo) != 1:
            raise fw.Abortado(
                "el parche de un control negativo no encontro su ancla exactamente una "
                "vez en %s (%d): %r. Sin inyectar el defecto, el control estaria "
                "aprobando el firmware sano y diciendo que sabe detectar"
                % (cual, cods[cual].count(viejo), viejo[:60]))
        cods[cual] = cods[cual].replace(viejo, nuevo)
    return cods


def correr(b, fw):
    b.titulo("El Modo Manual no cicla: la primera pulsacion entra y nada se mueve solo")

    cods = _lecturas(fw)
    try:
        d = _medir(cods["coord"], cods["manual"], cods["auto"])
    except ValueError as e:
        raise fw.Abortado(str(e))

    ent, ACEPTA = d["entradas"], d["ESTADO_ACEPTA"]
    b.verificar(
        True,
        "censo leido del C++: %d entradas de todo-rojo (%s), la peticion se acepta desde "
        "%s y el despeje se llama %s"
        % (len(ent), ", ".join("%s->%s" % kv for kv in sorted(ent.items())),
           ACEPTA, d["CTE_DESPEJE"]),
        "-")

    # ---- 1. Manual entra por una puerta que NO programa nada -------------------
    b.verificar(
        len(d["manual"]) == 1 and ent[d["manual"][0]] == ACEPTA,
        "modo_manual.cpp entra por %s(), que deja el coordinador en %s: la primera "
        "pulsacion de DAR PASO se acepta y no hay ningun cambio programado"
        % (d["manual"][0] if d["manual"] else "-", ACEPTA),
        "modo_manual.cpp entra por %s, que deja el coordinador en %s y no en %s. Las dos "
        "mitades del defecto salen de ahi: DAR PASO se rechaza mientras dure el plazo "
        "-pedirCambio() abre con 'if (estadoC != %s) return;'- y al vencer, el cruce "
        "cambia SOLO sin que nadie lo haya pedido"
        % (", ".join("%s()" % n for n in d["manual"]) or "ninguna entrada de todo-rojo",
           ", ".join(ent[n] for n in d["manual"]) or "-", ACEPTA, ACEPTA))

    # ---- 2. Y el Automatico SI programa el suyo (la mitad positiva) -----------
    #
    # Sin esta linea, la comprobacion 1 seria una TAPIA: un firmware en el que NINGUNA
    # entrada programase nada -o sea, uno al que se le hubiera quitado el todo-rojo de
    # apertura de SFTY-4- pasaria la 1 igual de bien que el correcto. Es 8.sexies: una
    # inversion sin el caso que exige que SI pase lo que debe pasar no mide nada.
    b.verificar(
        len(d["auto"]) == 1 and ent[d["auto"][0]] != ACEPTA,
        "modo_automatico.cpp entra por %s(), que SI deja programado su despeje de "
        "apertura (%s): el ciclo automatico arranca por todo-rojo, como exige SFTY-4"
        % (d["auto"][0] if d["auto"] else "-",
           ent[d["auto"][0]] if d["auto"] else "-"),
        "modo_automatico.cpp entra por %s. El modo que SI cicla tiene que programar su "
        "despeje de apertura; si no lo hace, la comprobacion 1 de aqui deja de medir una "
        "eleccion y aprueba cualquier cosa"
        % (", ".join("%s()->%s" % (n, ent[n]) for n in d["auto"]) or "ninguna"))

    # ---- 3. El despeje ya cumplido no se vuelve a cobrar ----------------------
    b.verificar(
        not re.search(r"\btRef\s*=", d["rama_parado"]),
        "la rama del cruce parado de %s() NO reinicia tRef: el rojo que ya lleva puesto "
        "cuenta, asi que pulsar dos veces no aleja el verde" % d["pedir"],
        "la rama del cruce parado de %s() reinicia tRef. Con el despeje en su valor de "
        "hoy, un operario que pulse mas rapido que ese plazo NO VE EL VERDE NUNCA, y cada "
        "pulsacion le contesta que si. Obedecer y no avanzar es la peor forma de fallar: "
        "no deja rastro de averia" % d["pedir"])

    # ---- 4. Pero el despeje SIGUE consultandose ------------------------------
    b.verificar(
        d["CTE_DESPEJE"] in d["rama_parado"],
        "y la misma rama sigue consultando %s antes de abrir paso: lo que se quito es "
        "cobrar dos veces el despeje, no el despeje" % d["CTE_DESPEJE"],
        "la rama del cruce parado de %s() ya no nombra %s: da verde sin mirar cuanto "
        "lleva el cruce en rojo. Eso no es 'inmediato para el operario', es saltarse el "
        "todo-rojo de SFTY-4 el dia que la pulsacion llegue pegada a un rojo recien puesto"
        % (d["pedir"], d["CTE_DESPEJE"]))

    # ---- Controles negativos: el defecto REAL, inyectado ---------------------
    def con(parches):
        try:
            return _medir(**{k: v for k, v in
                             zip(("cod_coord", "cod_manual", "cod_auto"),
                                 (lambda c: (c["coord"], c["manual"], c["auto"]))(
                                     _lecturas(fw, parches)))})
        except ValueError:
            return None

    # (a) El defecto tal como estaba: Manual llamando a la entrada del Automatico.
    entrada_auto = d["auto"][0] if d["auto"] else None
    entrada_man = d["manual"][0] if d["manual"] else None
    if entrada_auto and entrada_man:
        roto = con([("manual", "%s(" % entrada_man, "%s(" % entrada_auto)])
        b.control_negativo(
            roto is not None and ent[roto["manual"][0]] != ACEPTA,
            "con modo_manual.cpp llamando a %s() -el defecto tal como estaba- la "
            "comprobacion 1 cae" % entrada_auto)

    # (b) El tRef de vuelta en la rama del cruce parado.
    # El ancla es la RAMA ENTERA y no su primera linea: "if (millis() - tRef >= ..." sale
    # cuatro veces en el fichero -los tres case de espera y este-, y un ancla ambigua
    # parchearia otro sitio o abortaria. La rama completa solo existe una vez.
    ancla = d["rama_parado"]
    if ancla.strip():
        roto = con([("coord", ancla, " tRef = millis();" + ancla)])
        b.control_negativo(
            roto is not None and bool(re.search(r"\btRef\s*=", roto["rama_parado"])),
            "devuelto el 'tRef = millis()' a la rama del cruce parado, la "
            "comprobacion 3 cae")
