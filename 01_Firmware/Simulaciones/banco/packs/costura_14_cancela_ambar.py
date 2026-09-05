# ===== banco/packs/costura_14_cancela_ambar.py =====
#
# N-152 — EL ESCLAVO RETIRA SU AMBAR Y EL MAESTRO SE ENTERA. ES N-142 AL REVES.
#
# QUE PASABA, MEDIDO Y NO LEIDO
# -----------------------------
# En Esclavo/src/bluetooth.cpp la rama CANCELAR_AMBAR limpiaba el latch y contestaba al
# TELEFONO -RETIRADO o RETIRADO_QUEDA_MANDO-, y el UNICO protocolo_enviarPaquete() de
# todo el fichero era el del ARMADO (N-142). O sea: al enganchar el ambar se avisaba al
# Maestro y al quitarlo no.
#
# Y la consecuencia la CREO N-142, que es lo que la vuelve dificil de ver leyendo un
# fichero solo: desde el 04/09, al armar, el Maestro se va a MODO_AMBAR. Al cancelar
# desde el Poste 2, el Esclavo levanta su veto y espera la siguiente orden del Maestro,
# que NO LLEGA NUNCA porque en MODO_AMBAR el Maestro calla a proposito (SFTY-21). El
# cruce se quedaba en ambar hasta que alguien caminara hasta el otro poste.
#
# POR QUE UN PACK NUEVO Y NO UNA FILA MAS EN costura_13 (CLAUDE.md 2.bis)
# ----------------------------------------------------------------------
# costura_13 mide N-134 -que el Maestro ORDENE el ambar del Esclavo tras el todo-rojo-.
# Esto es la direccion contraria y otra maquina. Pero la razon de fondo no es de
# ordenacion: es que aqui hay UNA PROPIEDAD DE VIDA QUE NINGUN INSTRUMENTO EJERCIA.
#
#   EN MODO_AMBAR EL MAESTRO NO LEIA LA RADIO. Censado: protocolo_hayPaqueteDisponible()
#   se llamaba en UN solo sitio de todo el Maestro -coordinador_actualizar()- y main.cpp
#   excluye ese modo del refresco de fondo. Un CMD_CANCELA_AMBAR_ESCLAVO copiado de
#   N-142 habria entrado por el UART sin que nadie lo leyera: el arreglo entero habria
#   sido codigo DECLARADO Y NO EJERCIDO, con la compuerta en verde. Es la forma exacta
#   de los cinco defectos que pararon el banco del 3-4/09 (CLAUDE.md 2.ter).
#
# Esa propiedad -"existe un lector de radio alcanzable desde el modo en que el equipo se
# para"- es la comprobacion 2 y es la que justifica el fichero. Las otras tres cuelgan
# de ella: sin oido, ninguna significa nada.
#
# LAS CUATRO PROPIEDADES, Y NINGUNA ES "EXISTE EL COMANDO"
# -------------------------------------------------------
#   1. UN ACUSE QUE DICE QUE EL AMBAR SE FUE, SALE POR RADIO. Todo $ACK de
#      CANCELAR_AMBAR cuyo camino NO este sostenido por el latch del mando emite el
#      aviso; el que SI lo esta, no -esa punta sigue en ambar, y pedirle al Maestro que
#      salga del suyo deja el cruce descuadrado y al Maestro mandando ordenes que esta
#      punta no obedece ni acusa, que es el bloqueo que N-142 cerro-. Los $ERR tampoco
#      emiten: un rechazo no ha cambiado nada.
#   2. HAY OIDO EN AMBAR. Ver arriba.
#   3. TODO CAMINO AL AMBAR DECLARA SU ORIGEN. El aviso solo saca al Maestro del ambar
#      QUE PIDIO EL ESCLAVO; el que pidio una persona de este poste puede estar
#      protegiendo a quien esta en la calzada del Poste 1. Esa distincion se apoya en
#      una bandera, y una bandera que un camino nuevo se olvide de escribir es un
#      defecto con permiso: se exige que la funcion que mete al equipo en MODO_AMBAR
#      declare tambien de quien es.
#   4. LA SALIDA ES AL TODO-ROJO QUE NO PROGRAMA NADA. El Esclavo dijo "ya no retengo el
#      ambar", no "ya se puede pasar". Reanudar el ciclo daria verde a un cruce cuyo
#      Poste 1 no ha mirado nadie.
#
# EL BORDE CONTRA EL QUE MIDE CADA COMPROBACION, ESCRITO AL LADO (CLAUDE.md 4.quinquies)
# --------------------------------------------------------------------------------------
#   - "sostenido por el mando" = la guarda nombra al vetador SIN '!' delante. Con '!'
#     delante afirma lo contrario y por eso el reenvio -que lleva !mando- SI emite.
#   - "hay oido" = un bloque cuya condicion diga 'modo == MODO_AMBAR' -igualdad, no
#     desigualdad- llama a un lector de radio. La exclusion de SFTY-21 usa '!=' y por
#     eso no se confunde con ella.
#   - "declara su origen" = a nivel de FUNCION, no de bloque. Se mide asi a proposito:
#     en mando.cpp el modoActual_set() vive en el 'else' de un if y el motivo se fija
#     antes del if, asi que un borde de bloque diria que falta cuando esta. Lo que se
#     pierde con este borde: dos caminos al ambar dentro de la MISMA funcion, uno de los
#     cuales no declarara origen, pasarian. Hoy no existe ninguno.
#   - "el todo-rojo que no programa nada" = la funcion del coordinador que deja la
#     maquina en el estado que coordinador_pedirCambio() exige para aceptar una orden.
#     Ese estado se LEE de esa guarda, no se escribe aqui.
#
# LOS UNICOS NOMBRES ESCRITOS A MANO SON LOS DEL FICHERO Y LOS DEL LITERAL DE LA APP.
# El comando del aviso se DEDUCE -es el que emite la rama de CANCELAR_AMBAR-, el estado
# de reposo se DEDUCE, el todo-rojo se DEDUCE y el lector de radio se DEDUCE. Si algo de
# eso no se encuentra, el pack ABORTA: aprobar sin haber leido es fabricar un PASS.
#
# EJERCE SFTY-21: que salir de un ambar pedido por radio no levante el que pidio una
# persona de este poste, y que la salida sea al todo-rojo y no a un ciclo que da verde.

import re

from banco.packs.costura_13_ambar_ordenado import (   # lectores ya probados: se traen
    _cierre, _bloques_if, _definiciones, _atomos,     # enteros en vez de reescribirse
    _llamadas_stmt, _Parcheado,
)

NOMBRE = "costura_14_cancela_ambar"
DESCRIPCION = ("el Esclavo avisa por radio de que RETIRA su ambar, el Maestro lo oye "
               "estando en ambar y sale al todo-rojo solo si ese ambar era suyo (N-152)")

ESCLAVO_BT = ("Esclavo", "src", "bluetooth.cpp")
MAESTRO_MAIN = ("Maestro", "src", "main.cpp")
MAESTRO_COORD = ("Maestro", "src", "coordinador.cpp")

# El nombre del modo del ambar. Es el sujeto de la propiedad entera: si desaparece no
# hay nada que medir, y por eso su ausencia aborta en vez de aprobar.
MODO = "MODO_AMBAR"

# La orden que la app manda para retirar el ambar. Se escribe porque es un CONTRATO CON
# EL EXTERIOR -lo teclea la app, no el firmware- y deducirlo seria adivinar.
ORDEN = "CANCELAR_AMBAR"

# La unica API por la que el firmware lee la radio. Se nombra la funcion y no un
# prefijo: es una sola, y el pack existe precisamente porque en un modo no se llamaba.
LECTOR_RADIO = "protocolo_hayPaqueteDisponible"

RE_RESPUESTA = re.compile(
    r'"\$(ACK|ERR),CMD:%s,(?:RESULT|DESC):(\w+)"' % re.escape(ORDEN))
RE_EMISION = re.compile(r"\bprotocolo_enviarPaquete\s*\(\s*(CMD_\w+)")
RE_CASO_MODO = re.compile(r"case\s+(\w+)\s*:\s*(\w+)\s*\(\s*\)\s*;")
RE_FIJA_ESTADO = re.compile(r"\bestadoC\s*=\s*(C_\w+)\s*;")


# --------------------------------------------------------------------------------
# El unico lector propio: el bloque de llaves MAS INTERNO que contiene una posicion.
#
# Hace falta porque las tres salidas de CANCELAR_AMBAR viven en ramas 'else', y un
# 'else' no lo captura _bloques_if -que busca 'if ('-. Buscar por proximidad en vez de
# por llaves seria N-89 otra vez: el pack aprobaria por vecindad y no por pertenencia.
# --------------------------------------------------------------------------------

def _bloque_de(cod, pos):
    """(ini, fin) del '{...}' mas interno que envuelve a pos. None si no hay ninguno."""
    pila = []
    for j, c in enumerate(cod):
        if j >= pos:
            break
        if c == "{":
            pila.append(j)
        elif c == "}" and pila:
            pila.pop()
    if not pila:
        return None
    ini = pila[-1]
    fin = _cierre(cod, ini, "{", "}")
    return None if fin is None else (ini, fin)


# --------------------------------------------------------------------------------
# La medida. Una sola funcion, para que el firmware real y el parcheado pasen
# EXACTAMENTE por el mismo lector: si midieran por caminos distintos, el control
# negativo demostraria que funciona otra cosa.
# --------------------------------------------------------------------------------

def _medir(fw):
    d = {}

    # =============== ESCLAVO: quien avisa, y desde donde ==========================
    bt = fw.codigo(*ESCLAVO_BT)
    bloques_bt = _bloques_if(bt)

    # Los vetadores: las funciones que el despachador de radio del Esclavo consulta
    # NEGADAS para decidir si obedece. Se deducen de main.cpp y no se escriben: son las
    # que sostienen el ambar, y una de ellas es el latch del mando.
    main_e = fw.codigo("Esclavo", "src", "main.cpp")
    vetadores = set()
    for cond, _i, _f in _bloques_if(main_e):
        atomos = _atomos(cond)
        if len(atomos) < 2:
            continue
        negadas = {re.sub(r"^!\s*", "", a) for a in atomos if a.startswith("!")}
        negadas = {a for a in negadas if re.fullmatch(r"\w+\s*\(\s*\)", a)}
        if len(negadas) >= 2:
            vetadores |= {re.sub(r"\s", "", a) for a in negadas}
    if len(vetadores) < 2:
        raise fw.Abortado(
            "en Esclavo/src/main.cpp no se hallo ninguna guarda con DOS getters negados "
            "en la misma condicion (halladas: %s). Ese par es el veto del ambar -el del "
            "mando y el de la app- y es contra el que se decide desde que salida de %s "
            "sale el aviso. Sin poder leerlo habria que escribir el nombre del latch a "
            "mano, que es el valor por defecto que este banco no admite"
            % (sorted(vetadores), ORDEN))
    d["vetadores"] = sorted(vetadores)

    # Las salidas de la orden, por su literal de respuesta. Cada una con su bloque.
    salidas = {}
    for m in RE_RESPUESTA.finditer(bt):
        blq = _bloque_de(bt, m.start())
        if blq is None:
            raise fw.Abortado(
                "la respuesta %r de %s no esta dentro de ningun bloque de llaves. El "
                "pack mide POR PERTENENCIA, no por cercania; sin bloque no puede decir "
                "si el aviso sale de ahi o de al lado" % (m.group(2), ORDEN))
        ini, fin = blq
        guardas = {a for cond, i, f in bloques_bt if i <= m.start() < f
                   for a in _atomos(cond)}
        # El borde: "sostenido por el mando" es nombrar al vetador SIN '!' delante.
        sostenido = any(v in re.sub(r"\s", "", a) and not a.strip().startswith("!")
                        for a in guardas for v in vetadores)
        salidas[m.group(2)] = {
            "tipo": m.group(1),
            "emite": set(RE_EMISION.findall(bt[ini:fin])),
            "sostenido": sostenido,
        }
    if len(salidas) < 3:
        raise fw.Abortado(
            "solo se hallaron %d salida(s) de CMD:%s en Esclavo/src/bluetooth.cpp (%s). "
            "El firmware tiene al menos el acuse, el acuse con el mando puesto y el "
            "rechazo; con menos, el pack estaria midiendo un conjunto casi vacio, y un "
            "conjunto vacio aprueba cualquier cosa"
            % (len(salidas), ORDEN, sorted(salidas)))
    d["salidas"] = salidas

    # El comando del aviso SE DEDUCE: el que emiten las salidas de esta orden.
    emitidos = set()
    for s in salidas.values():
        emitidos |= s["emite"]
    if len(emitidos) != 1:
        raise fw.Abortado(
            "las salidas de CMD:%s del Esclavo emiten %d comando(s) distinto(s) por "
            "radio (%s). Con NINGUNO el Maestro no se entera de que el ambar se retiro "
            "-que es N-152- y con VARIOS hay dos avisos para el mismo hecho: el resto "
            "del pack no sabria cual seguir" % (ORDEN, len(emitidos), sorted(emitidos)))
    d["CMD_AVISO"] = sorted(emitidos)[0]

    # Cada salida en su sitio: acuse no sostenido -> emite; sostenido o rechazo -> no.
    d["deben_emitir"] = sorted(n for n, s in salidas.items()
                               if s["tipo"] == "ACK" and not s["sostenido"])
    d["no_deben_emitir"] = sorted(n for n, s in salidas.items()
                                  if s["tipo"] != "ACK" or s["sostenido"])
    d["mudas_debiendo"] = sorted(n for n in d["deben_emitir"] if not salidas[n]["emite"])
    d["hablan_no_debiendo"] = sorted(n for n in d["no_deben_emitir"]
                                     if salidas[n]["emite"])

    # =============== MAESTRO: que lo oiga estando en ambar ========================
    # Los lectores de radio: censando el DIRECTORIO, no una lista escrita a mano.
    lectores = set()
    for f_ in fw.fuentes_de("Maestro", "src"):
        cod = fw.codigo("Maestro", "src", f_)
        for n, i, f in _definiciones(cod):
            if LECTOR_RADIO in cod[i:f]:
                lectores.add(n)
    if not lectores:
        raise fw.Abortado(
            "en Maestro/src no hay ninguna funcion que llame a %s(). Ese es el unico "
            "sitio por el que entra una trama; sin encontrarlo el pack no puede decir "
            "si el Maestro oye en algun modo, y aprobaria por no haber mirado"
            % LECTOR_RADIO)
    d["lectores"] = sorted(lectores)

    main_m = fw.codigo(*MAESTRO_MAIN)
    bloques_m = _bloques_if(main_m)

    # El borde: IGUALDAD con el modo, no desigualdad. La exclusion de SFTY-21 se escribe
    # con '!=' y meterla aqui haria pasar la comprobacion sobre el firmware sordo.
    re_igual = re.compile(r"==\s*%s\b" % re.escape(MODO))
    d["oyentes"] = sorted(
        {n for cond, i, f in bloques_m if re_igual.search(cond)
         for n in _llamadas_stmt(main_m[i:f]) if n in lectores})

    # =============== MAESTRO: de quien es el ambar, y por donde se sale ===========
    # El getter del origen y sus declaradores, deducidos de modo_ambar.cpp: las
    # funciones que ASIGNAN la bandera que el getter devuelve.
    amb = fw.codigo("Maestro", "src", "modo_ambar.cpp")
    m_get = re.search(r"\bbool\s+(\w+)\s*\(\s*\)\s*\{\s*return\s+(\w+)\s*;\s*\}", amb)
    if not m_get:
        raise fw.Abortado(
            "en Maestro/src/modo_ambar.cpp no hay ningun getter 'bool f() { return x; }'. "
            "Ese getter es lo que distingue el ambar que pidio el Esclavo del que pidio "
            "una persona de este poste, y sin el la cancelacion por radio levantaria "
            "cualquier ambar -incluido el que protege a quien esta en la calzada-")
    d["getter_origen"], bandera = m_get.group(1), m_get.group(2)
    re_asigna = re.compile(r"\b%s\s*=" % re.escape(bandera))
    d["declaradores"] = sorted(n for n, i, f in _definiciones(amb)
                               if re_asigna.search(amb[i:f]))
    if len(d["declaradores"]) < 2:
        raise fw.Abortado(
            "solo %d funcion(es) de modo_ambar.cpp escribe(n) la bandera de origen (%s). "
            "Hacen falta al menos dos -la que dice 'lo pidio el Esclavo' y la que dice "
            "'lo pidio alguien de aqui'-; con una sola la bandera no puede volver a "
            "false y la distincion seria decorativa"
            % (len(d["declaradores"]), d["declaradores"]))

    # Toda funcion de Maestro/src que meta al equipo en el modo declara su origen.
    # El borde es la FUNCION, y el porque esta escrito en la cabecera.
    re_entra = re.compile(r"\bmodoActual_set\s*\(\s*%s\s*\)" % re.escape(MODO))
    entradas, mudas = [], []
    for f_ in fw.fuentes_de("Maestro", "src"):
        cod = fw.codigo("Maestro", "src", f_)
        for n, i, f in _definiciones(cod):
            if not re_entra.search(cod[i:f]):
                continue
            entradas.append("%s/%s()" % (f_, n))
            # OJO AL BUSCADOR, y costo un FALLA falso: aqui NO vale _llamadas_stmt().
            # Su patron excluye parentesis dentro de los argumentos y los motivos los
            # llevan -"el mando (B.B.B)", "la app (celular)"-, asi que daba por MUDAS a
            # las dos funciones que si declaran. Un "no aparece" no es un hallazgo hasta
            # haber descartado al buscador (CLAUDE.md 4).
            if not any(re.search(r"\b%s\s*\(" % re.escape(dec), cod[i:f])
                       for dec in d["declaradores"]):
                mudas.append("%s/%s()" % (f_, n))
    if not entradas:
        raise fw.Abortado(
            "no se hallo en Maestro/src ninguna funcion con modoActual_set(%s). Si nadie "
            "entra al modo, la propiedad que este pack vigila no tiene sujeto" % MODO)
    d["entradas"], d["entradas_mudas"] = sorted(entradas), sorted(mudas)

    # La guarda que consume el aviso, y el modo al que sale.
    consumo = [(cond, i, f) for cond, i, f in bloques_m
               if d["getter_origen"] in cond]
    if len(consumo) != 1:
        raise fw.Abortado(
            "en Maestro/src/main.cpp hay %d guarda(s) que consulten %s(). Esa guarda es "
            "la que decide si la cancelacion del Esclavo se obedece: con ninguna el "
            "aviso no hace nada y con varias hay dos criterios para la misma decision"
            % (len(consumo), d["getter_origen"]))
    cond_c, ini_c, fin_c = consumo[0]
    d["atomos_consumo"] = sorted(_atomos(cond_c))
    d["exige_el_modo"] = any(re_igual.search(a) for a in d["atomos_consumo"])
    destinos = re.findall(r"\bmodoActual_set\s*\(\s*(\w+)\s*\)", main_m[ini_c:fin_c])
    if len(set(destinos)) != 1:
        raise fw.Abortado(
            "la guarda que consume la cancelacion lleva %d destino(s) de modo (%s). Si "
            "no sale a ninguno, el aviso se consume y el cruce sigue en ambar -el "
            "defecto entero-; si sale a varios, no hay 'el destino' que comprobar"
            % (len(set(destinos)), sorted(set(destinos))))
    d["destino"] = destinos[0]

    # El despacho de ARRANQUE, que NO es el unico: main.cpp tiene dos switch sobre el
    # mismo modo -uno de setup y otro de loop- y los dos casan con el mismo patron. El
    # de arranque es el que vive DENTRO del if del flanco de cambio de modo; el de loop
    # esta suelto. Quedarse con el ultimo que se lea daba modoManual_loop() en vez de
    # modoManual_setup(), y con el la comprobacion 4 aprobaba midiendo otra cosa: lo
    # descubrio su propio control negativo, no una lectura.
    casos = {}
    for cond, i, f in bloques_m:
        for m in RE_CASO_MODO.finditer(main_m[i:f]):
            casos.setdefault(m.group(1), m.group(2))
    if d["destino"] not in casos:
        raise fw.Abortado(
            "el modo de destino (%s) no aparece en ningun despacho 'case MODO: f();' "
            "DENTRO de un if de main.cpp (leidos: %s). Ese es el arranque del modo; el "
            "switch suelto de mas abajo es el del loop, y confundirlos hace que la "
            "comprobacion 4 mida la funcion equivocada"
            % (d["destino"], sorted(casos)))
    d["casos"] = casos
    d["setup_destino"] = casos[d["destino"]]

    # El estado de REPOSO se lee de la guarda de coordinador_pedirCambio(): es el estado
    # desde el que la maquina acepta una orden. No se escribe aqui.
    coord = fw.codigo(*MAESTRO_COORD)
    defs_coord = {n: (i, f) for n, i, f in _definiciones(coord)}
    pedir = [n for n in defs_coord if "pedirCambio" in n]
    if len(pedir) != 1:
        raise fw.Abortado(
            "en coordinador.cpp hay %d funcion(es) '*pedirCambio*' (%s). De su guarda se "
            "lee cual es el estado de reposo, y sin ese nombre no se puede distinguir el "
            "todo-rojo que PARA del que programa un verde" % (len(pedir), sorted(pedir)))
    i_p, f_p = defs_coord[pedir[0]]
    m_rep = re.search(r"\bestadoC\s*!=\s*(C_\w+)", coord[i_p:f_p])
    if not m_rep:
        raise fw.Abortado(
            "%s() ya no rechaza por 'estadoC != C_X'. Esa comparacion es de donde se lee "
            "el estado de reposo; escribirlo a mano aqui seria el valor por defecto que "
            "este banco no admite" % pedir[0])
    d["reposo"] = m_rep.group(1)

    # Los todo-rojos del coordinador: ponen la luz local en rojo Y mandan orden por
    # radio. Se clasifican por el estado en que DEJAN la maquina.
    # UN PASO deja la maquina en UN estado, y ese es el borde: una funcion con varias
    # asignaciones de estadoC no es un paso, es la maquina -coordinador_actualizar() y
    # pedirCambio() entraban aqui, y una acababa clasificada como "para" por cual fuera
    # su ULTIMA asignacion textual, que no es donde termina la ejecucion-.
    paran, programan = [], []
    for n, i, f in _definiciones(coord):
        cuerpo = coord[i:f]
        if "semaforo_forzarRojo" not in _llamadas_stmt(cuerpo):
            continue
        if not RE_EMISION.search(cuerpo):
            continue
        estados = RE_FIJA_ESTADO.findall(cuerpo)
        if len(estados) != 1:
            continue
        (paran if estados[0] == d["reposo"] else programan).append(n)
    if not paran or not programan:
        raise fw.Abortado(
            "el censo de todo-rojos del coordinador dio %d que PARAN y %d que PROGRAMAN "
            "(%s / %s). Hacen falta los dos grupos: sin el que programa, la comprobacion "
            "4 no puede distinguir la salida segura de la que da verde, y aprobaria las "
            "dos" % (len(paran), len(programan), sorted(paran), sorted(programan)))
    d["paran"], d["programan"] = sorted(paran), sorted(programan)

    # Que llama el setup del destino.
    llam_setup = None
    for f_ in fw.fuentes_de("Maestro", "src"):
        cod = fw.codigo("Maestro", "src", f_)
        for n, i, f in _definiciones(cod):
            if n == d["setup_destino"]:
                llam_setup = _llamadas_stmt(cod[i:f])
    if llam_setup is None:
        raise fw.Abortado(
            "no se hallo la definicion de %s() en Maestro/src. Es el arranque del modo "
            "al que sale el cruce; sin leerlo no se sabe con que empieza"
            % d["setup_destino"])
    d["sale_parando"] = sorted(llam_setup & set(d["paran"]))
    d["sale_programando"] = sorted(llam_setup & set(d["programan"]))

    return d


# --------------------------------------------------------------------------------
# El lector con el defecto inyectado EN MEMORIA (CLAUDE.md 8.bis).
#
# Los .cpp reales NO se tocan aqui: un arnes que edita el firmware para probarse deja el
# arbol sucio si algo revienta a mitad. La inyeccion sobre el .cpp REAL se hizo aparte,
# a mano, y su salida esta en el informe de N-152.
#
# LAS ANCLAS SE EXTRAEN DEL FUENTE DE ESTA MISMA CORRIDA -son rodajas exactas medidas
# por _medir()-, asi que no pueden caducar en silencio. Y si un parche no encuentra la
# suya, _con_defecto() ABORTA: un ancla caducada dejaria el control negativo "fallando
# bien" sobre el firmware SANO, y su verde no valdria nada.
# --------------------------------------------------------------------------------

def _con_defecto(fw, parches):
    """Mide sobre el firmware parcheado y exige que TODOS los parches se aplicaran."""
    p = _Parcheado(fw, parches)
    try:
        med = _medir(p)
    except fw.Abortado:
        # Un parche que deja el fuente ilegible NO es "detecto el defecto": es que el
        # pack dejo de saber leer. Se distingue devolviendo None.
        med = None
    pedidos = {(k, a) for k, v in parches.items() for a, _ in v}
    if p.aplicados != pedidos:
        raise fw.Abortado(
            "el control negativo pidio %d parche(s) y solo entro(aron) %d. Un parche que "
            "nunca llega a aplicarse deja el control negativo aprobando sobre el firmware "
            "SANO, y su verde diria que el pack sabe detectar un defecto que jamas se "
            "inyecto" % (len(pedidos), len(p.aplicados)))
    return med


def _rodaja_salida(fw, nombre):
    """El texto exacto del bloque de una salida de CANCELAR_AMBAR. Es el ancla."""
    bt = fw.codigo(*ESCLAVO_BT)
    for m in RE_RESPUESTA.finditer(bt):
        if m.group(2) == nombre:
            ini, fin = _bloque_de(bt, m.start())
            return bt[ini:fin + 1]
    raise fw.Abortado(
        "el control negativo no encontro la salida %r de CMD:%s para inyectarle el "
        "defecto. El ancla se saca del fuente real en esta misma corrida, asi que no "
        "hallarla significa que el lector cambio de idea entre dos lecturas"
        % (nombre, ORDEN))


def correr(b, fw):
    b.titulo("N-152: el Esclavo retira su ambar, y el Maestro se entera")

    d = _medir(fw)

    # El censo se publica pero NO cuenta: es la medida sobre la que se apoyan las
    # comprobaciones, no una comprobacion. Contarlo inflaria el total con una linea que
    # ningun firmware puede fallar.
    b.reportar(
        "deducido del firmware, sin un solo nombre de comando escrito en el pack",
        ["aviso de retirada (emitido por la rama de %s): %s" % (ORDEN, d["CMD_AVISO"]),
         "vetadores del ambar en el Esclavo:      %s" % ", ".join(d["vetadores"]),
         "salidas de %-16s        %s" % (ORDEN + ":", ", ".join(
             "%s[%s%s]" % (n, s["tipo"], ",mando" if s["sostenido"] else "")
             for n, s in sorted(d["salidas"].items()))),
         "lectores de radio del Maestro:          %s" % ", ".join(d["lectores"]),
         "getter del origen del ambar:            %s()" % d["getter_origen"],
         "quien declara ese origen:               %s"
         % ", ".join("%s()" % n for n in d["declaradores"]),
         "funciones que entran en %s:     %s" % (MODO, ", ".join(d["entradas"])),
         "estado de reposo del coordinador:       %s" % d["reposo"],
         "todo-rojos que PARAN / que PROGRAMAN:   %s / %s"
         % (", ".join(d["paran"]), ", ".join(d["programan"])),
         "salida del ambar: %s -> %s()" % (d["destino"], d["setup_destino"])])

    # ---- 1. Un acuse que dice que el ambar se fue, sale por radio ----------------
    b.verificar(
        not d["mudas_debiendo"] and not d["hablan_no_debiendo"],
        "las %d salida(s) de CMD:%s que declaran el ambar retirado emiten %s por radio "
        "(%s), y las %d que no lo declaran callan (%s): el Maestro se entera cuando el "
        "ambar se va de verdad, y no cuando solo se quita uno de los dos latches"
        % (len(d["deben_emitir"]), ORDEN, d["CMD_AVISO"], ", ".join(d["deben_emitir"]),
           len(d["no_deben_emitir"]), ", ".join(d["no_deben_emitir"])),
        "SFTY: el aviso de retirada del ambar no sale por donde debe. Salidas que "
        "declaran el ambar retirado y NO avisan al Maestro: %s -es N-152: el Maestro se "
        "queda en MODO_AMBAR, donde CALLA, asi que no hay siguiente orden que devuelva "
        "el cruce y hay que caminar hasta el otro poste-. Salidas que avisan sin haber "
        "retirado nada: %s -el Maestro saldria de su ambar mientras esta punta sigue en "
        "el suyo, y le mandaria ordenes que aqui no se obedecen ni se acusan-"
        % (d["mudas_debiendo"] or "ninguna", d["hablan_no_debiendo"] or "ninguna"))

    # ---- 2. Y hay quien lo oiga: en ambar el Maestro escucha aunque calle ---------
    b.verificar(
        bool(d["oyentes"]),
        "en %s el Maestro LEE la radio: %s se llama desde un bloque guardado por '== %s' "
        "(lectores hallados: %s). Callar es no transmitir, no quedarse sordo: sin esto el "
        "aviso entraria por el UART y no lo leeria nadie"
        % (MODO, ", ".join("%s()" % n for n in d["oyentes"]), MODO,
           ", ".join(d["lectores"])),
        "EL MAESTRO ESTA SORDO EN %s: ningun bloque guardado por '== %s' llama a un "
        "lector de radio (%s). El aviso de que el Esclavo retiro su ambar llega al UART "
        "y no lo lee nadie, asi que todo lo demas de N-152 es codigo declarado y no "
        "ejercido: el cruce se queda en ambar igual que antes, pero ahora con un pack "
        "verde encima" % (MODO, MODO, ", ".join(d["lectores"])))

    # ---- 3. Todo camino al ambar declara de quien es ------------------------------
    b.verificar(
        not d["entradas_mudas"] and d["exige_el_modo"],
        "las %d funcion(es) que meten al equipo en %s declaran tambien su origen (%s), y "
        "la guarda que consume la cancelacion exige el modo ademas del origen (%s): un "
        "ambar pedido desde este poste no lo levanta el otro extremo"
        % (len(d["entradas"]), MODO, ", ".join(d["entradas"]),
           " && ".join(d["atomos_consumo"])),
        "SFTY: la distincion entre 'este ambar lo pidio el Esclavo' y 'lo pidio alguien "
        "de este poste' tiene un agujero. Funciones que entran en %s sin declarar "
        "origen: %s. Guarda del consumo: %s (exige el modo: %s). Por ese agujero, un "
        "aviso del otro extremo saca al cruce de un ambar que puso una persona -que "
        "puede estar en la calzada del Poste 1-, y eso es peor que el defecto que N-152 "
        "arregla" % (MODO, d["entradas_mudas"] or "ninguna",
                     " && ".join(d["atomos_consumo"]), d["exige_el_modo"]))

    # ---- 4. Y se sale al todo-rojo que no programa nada ---------------------------
    b.verificar(
        bool(d["sale_parando"]) and not d["sale_programando"],
        "al retirarse el ambar el cruce sale a %s, cuyo arranque %s() llama a %s -el "
        "todo-rojo que deja la maquina en %s, o sea parada y sin plazo- y a ninguno de "
        "los que programan un cambio (%s): el Esclavo dijo que ya no retiene el ambar, "
        "no que se pueda pasar"
        % (d["destino"], d["setup_destino"],
           ", ".join("%s()" % n for n in d["sale_parando"]), d["reposo"],
           ", ".join(d["programan"])),
        "SFTY: la salida del ambar por cancelacion del Esclavo NO para el cruce. %s() "
        "llama a %s, que programa un cambio en vez de dejar la maquina en %s. Reanudar "
        "el ciclo da verde a un cruce cuyo Poste 1 no ha mirado nadie, y el aviso solo "
        "decia que la otra punta dejo de retener el ambar"
        % (d["setup_destino"], d["sale_programando"] or "ningun todo-rojo", d["reposo"]))

    _controles(b, fw, d)


def _controles(b, fw, d):
    b.titulo("Controles negativos: la prueba sabe fallar")

    # --- 1. N-152 mismo: la salida limpia deja de avisar --------------------------
    limpia = sorted(n for n in d["deben_emitir"] if not d["salidas"][n]["sostenido"])
    rod = _rodaja_salida(fw, limpia[0])
    med = _con_defecto(fw, {ESCLAVO_BT: [(rod, rod.replace(
        "protocolo_enviarPaquete(%s);" % d["CMD_AVISO"], ";", 1))]})
    b.control_negativo(
        med is not None and med["mudas_debiendo"] == [limpia[0]],
        "quitandole el aviso a la salida %r -que es el defecto N-152 tal cual estaba el "
        "05/09- la comprobacion 1 cae y lo nombra" % limpia[0])

    # --- 2. Y la direccion contraria: avisar con el mando todavia puesto -----------
    sostenidas = sorted(n for n in d["no_deben_emitir"] if d["salidas"][n]["sostenido"])
    if sostenidas:
        rod = _rodaja_salida(fw, sostenidas[0])
        med = _con_defecto(fw, {ESCLAVO_BT: [(rod, rod.replace(
            "enviarTramaConCrc",
            "protocolo_enviarPaquete(%s); enviarTramaConCrc" % d["CMD_AVISO"], 1))]})
        b.control_negativo(
            med is not None and med["hablan_no_debiendo"] == [sostenidas[0]],
            "y al reves: avisando desde %r -la salida con el latch del mando todavia "
            "puesto- la misma comprobacion cae. No mide 'que haya aviso': mide de DONDE "
            "sale" % sostenidas[0])
    else:
        b.control_negativo(
            False,
            "no hay ninguna salida de CMD:%s sostenida por el mando contra la que "
            "ejercer la direccion contraria: la comprobacion 1 solo mediria una mitad"
            % ORDEN)

    # --- 3. El oido: se le quita la llamada al lector en el modo -------------------
    oyente = d["oyentes"][0]
    med = _con_defecto(fw, {MAESTRO_MAIN: [("%s();" % oyente, ";")]})
    b.control_negativo(
        med is not None and not med["oyentes"],
        "quitando la llamada a %s() de main.cpp, la comprobacion 2 cae: es el defecto "
        "que un N-152 copiado de N-142 habria dejado dentro -trama que llega y nadie "
        "lee- y ninguna otra comprobacion lo veria" % oyente)

    # --- 4. La salida: al ciclo en vez de al todo-rojo -----------------------------
    # El destino de la inyeccion NO se escribe: es el modo cuyo arranque llama a un
    # todo-rojo que NO deja la maquina en reposo -o sea, el que reanuda-. Escribir
    # "MODO_AUTOMATICO" aqui seria dar por sabido justo lo que la comprobacion mide.
    reanudan = []
    for modo_, setup_ in sorted(d["casos"].items()):
        if modo_ == d["destino"]:
            continue
        for f_ in fw.fuentes_de("Maestro", "src"):
            cod = fw.codigo("Maestro", "src", f_)
            for n, i, f in _definiciones(cod):
                if n == setup_ and (_llamadas_stmt(cod[i:f]) & set(d["programan"])):
                    reanudan.append(modo_)
    if reanudan:
        cambio = ("modoActual_set(%s);" % d["destino"],
                  "modoActual_set(%s);" % reanudan[0])
        med = _con_defecto(fw, {MAESTRO_MAIN: [cambio]})
        b.control_negativo(
            med is not None and (med["sale_programando"] or not med["sale_parando"]),
            "y mandando la salida a %s -cuyo arranque SI programa un cambio- en vez de a "
            "%s, la comprobacion 4 cae: distingue el todo-rojo que PARA del que programa "
            "un verde, que es la diferencia entre parar el cruce y reanudarlo sin que "
            "nadie haya mirado el Poste 1" % (reanudan[0], d["destino"]))
    else:
        b.control_negativo(
            False,
            "no se hallo ningun modo cuyo arranque programe un cambio contra el que "
            "ejercer la comprobacion 4: sin un caso malo que ensenarle, su PASS no "
            "distingue nada")

    b.reportar(
        "LO QUE ESTE PACK NO MIDE",
        ["- Que la trama LLEGUE. Se mide que sale, que hay quien la oiga y que quien la",
         "  oye decide bien. Que la radio la entregue no se ve desde el PC: la red para",
         "  cuando NO llega es la segunda pulsacion del operario, y eso lo mide la",
         "  comprobacion 1 sobre la salida de reenvio, no un arnes.",
         "- El ORDEN dentro de la vuelta de loop(). El aviso se anota en una vuelta y se",
         "  consume en la siguiente; son milisegundos y no se comprueba.",
         "- Dos caminos al ambar dentro de la MISMA funcion, uno sin declarar origen.",
         "  El borde de la comprobacion 3 es la funcion, y esta escrito arriba.",
         "- Que el Maestro NO responda mientras escucha en ambar. Se mide que oye; que",
         "  siga callado lo vigila SFTY-21 desde costura_08."])
