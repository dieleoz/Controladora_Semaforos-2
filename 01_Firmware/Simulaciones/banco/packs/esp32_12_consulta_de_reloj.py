# ===== banco/packs/esp32_12_consulta_de_reloj.py =====
#
# LA CONSULTA DE RELOJ: QUE NO INVENTE UNA HORA, Y QUE NADIE MAS LA CONTESTE.
#
# LA PROPIEDAD, EN DOS LINEAS:
#
#   1. Cada valor del enum MotivoSinHora tiene su propia rama con su propio texto en el
#      despachador del puente, y la hora solo sale detras de reloj_leer().
#   2. Lo que el puente SE QUEDA -lo que despachador_esParaElPuente() reclama- no lo
#      atiende NINGUNA punta del STM32. Ni hoy ni el dia que alguien lo escriba.
#
# POR QUE EXISTE, Y ES LA MITAD QUE FALTABA DE UNA DECISION YA TOMADA.
#
# ESP32_Expansion/include/despachador.h razona con todas las letras por que LEER_RTC no
# sigue viaje hacia el STM32, y lo razona con una medida sobre el bluetooth.cpp compilado
# de las dos puntas:
#
#     entrada: CMD:LEER_RTC
#     MAESTRO -> $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO
#     ESCLAVO -> $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO
#
# Reenviarlo le acusaria al operario de teclear mal una clave que no ha tecleado, CADA
# VEZ QUE PREGUNTA LA HORA. Es el mismo defecto por el que "$LATIDO" tiene rama muda en
# el Maestro, y el que D-15 acaba de cerrar en SET_RTC: dos acuses a una sola orden.
#
# 🔴 PERO ESA CABECERA AFIRMABA ADEMAS QUE UN PACK LO VIGILABA, Y NO EXISTIA. Medido:
#
#     grep -rn "LEER_RTC\|esParaElPuente" 01_Firmware/Simulaciones --include=*.py
#     -> cero coincidencias
#
# O sea: un mecanismo declarado, documentado con su motivo, y sin nada que lo ejecute.
# Es N-73 -la Caja Negra de Alarmas- y es sobre todo N-122: "un huerfano se acepta por
# una razon, y una razon es una AFIRMACION SOBRE EL CODIGO -o sea, algo que se comprueba,
# no que se escribe". Este fichero es esa comprobacion. Sin el, el veto del puente seria
# exactamente lo que este repositorio castiga: una lista de excepciones con un motivo
# escrito que nadie recalcula.
#
# LO QUE MIDE Y LO QUE NO. Esto es Python leyendo .cpp: NO ejecuta el firmware, NO
# enciende un ESP32 y NO ha visto un solo registro del DS3231. Que la hora que sale sea
# la de verdad solo lo puede decir el banco. Aqui se comprueba que el CAMINO existe, que
# ningun motivo se queda mudo, y que nadie mas contesta por la misma orden.
#
# SIN ETIQUETA SFTY, Y ES DELIBERADO. Toca el reloj, y de un reloj cuelga el Modo
# Degradado -SFTY-18-, pero esto NO EJERCE esa regla: no mide un enclavamiento, ni un
# umbral, ni una maniobra. Comprueba la forma de un despachador de consulta. Una regla
# que aparece cubierta por una prueba que no la ejerce es peor que una fila vacia, porque
# la vacia no miente.

import re

NOMBRE = "esp32_12_consulta_de_reloj"
DESCRIPCION = ("la consulta de reloj no inventa hora, contesta cada motivo por su nombre, "
               "y lo que el puente se queda no lo atiende ninguna punta")

ROL = "ESP32_Expansion"
DESPACHADOR = ("ESP32_Expansion", "src", "despachador.cpp")
CABECERA_RELOJ = ("ESP32_Expansion", "include", "reloj_ds3231.h")
CABECERA_DESP = ("ESP32_Expansion", "include", "despachador.h")
PUENTE_CPP = ("ESP32_Expansion", "src", "puente.cpp")
PUNTAS = ("Maestro", "Esclavo")

# El predicado que decide que se queda. Se nombra aqui porque es el contrato; lo que NO
# se escribe a mano es ninguno de los comandos que reclama.
PREDICADO = "despachador_esParaElPuente"


def _bloque(texto, i):
    """El contenido del bloque { ... } que abre en la posicion i."""
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


def _cuerpo(codigo, firma):
    m = re.search(firma + r"\s*\{", codigo)
    return None if not m else _bloque(codigo, m.end() - 1)


def _bloque_que_contiene(codigo, idx):
    """El bloque { ... } mas interno que envuelve a la posicion idx.

    Bloque literal de reloj_01_consulta_por_bluetooth._bloque_que_contiene(): es el
    codigo ya probado que mide "dentro de SU rama" en vez de "cerca en el fichero", y
    reescribirlo para renombrar una variable es como se cuelan los errores en un cambio
    que no debe cambiar comportamiento."""
    prof, ini = 0, -1
    for j in range(idx - 1, -1, -1):
        c = codigo[j]
        if c == "}":
            prof += 1
        elif c == "{":
            if prof == 0:
                ini = j
                break
            prof -= 1
    if ini < 0:
        return None
    prof = 0
    for j in range(ini, len(codigo)):
        if codigo[j] == "{":
            prof += 1
        elif codigo[j] == "}":
            prof -= 1
            if prof == 0:
                return codigo[ini + 1:j]
    return None


def _valores_del_enum(codigo, nombre):
    m = re.search(r"\benum\s+%s\s*\{" % re.escape(nombre), codigo)
    if not m:
        return []
    cuerpo = _bloque(codigo, m.end() - 1)
    if cuerpo is None:
        return []
    return re.findall(r"\b([A-Z][A-Z0-9_]{2,})\b", cuerpo)


def _reclamados(codigo):
    """Los comandos que el puente se QUEDA, leidos del predicado y de sus constantes.

    No hay lista escrita a mano: el predicado compara contra constantes y las
    constantes llevan el texto que viaja por el cable. Si alguien anade un comando
    exclusivo del puente, aparece aqui solo -y las comprobaciones de abajo se le
    aplican sin que nadie se acuerde de anadirlo-."""
    cuerpo = _cuerpo(codigo, r"bool\s+%s\s*\([^)]*\)" % re.escape(PREDICADO))
    if cuerpo is None:
        return None, None
    # Las constantes que el predicado nombra, y su literal.
    consts = dict(re.findall(
        r'static\s+const\s+char\s+(\w+)\[\]\s*=\s*"([^"]*)"', codigo))
    usadas = [n for n in consts if re.search(r"\b%s\b" % re.escape(n), cuerpo)]
    return cuerpo, {n: consts[n] for n in usadas}


def correr(b, fw):
    b.titulo("La consulta de reloj: cada 'no se' con su nombre, y una sola respuesta")

    desp = fw.codigo(*DESPACHADOR)
    cabecera = fw.codigo(*CABECERA_RELOJ)

    # =====================================================================
    b.titulo("1. La orden que el puente se queda, leida del predicado")
    # =====================================================================
    cuerpoPred, reclamados = _reclamados(desp)
    if cuerpoPred is None:
        raise fw.Abortado(
            "no se hallo %s() en %s/src/despachador.cpp. Es el unico sitio donde se "
            "decide que linea NO cruza el cable: sin poder leerlo, este pack aprobaria "
            "un veto que no ha visto" % (PREDICADO, ROL))
    if not reclamados:
        raise fw.Abortado(
            "%s() existe y no nombra ni una constante con un literal de comando. O el "
            "predicado cambio de forma -y compara contra otra cosa- o el buscador se "
            "quedo ciego: en los dos casos, medir un conjunto vacio de ordenes "
            "reclamadas aprueba cualquier cosa" % PREDICADO)

    b.verificar(
        True,
        "el puente se queda %d orden(es), leida(s) del predicado: %s"
        % (len(reclamados), sorted(reclamados.values())),
        "no deberia llegarse aqui")

    # El predicado compara ENTERA, no por subcadena. Un strstr aqui haria que el puente
    # se quedara "CMD:PIN:1234:LEER_RTC" y cualquier otra linea con el texto dentro: o
    # sea, ordenes que no son suyas desapareciendo del cable sin que nada lo diga.
    b.verificar(
        "strcmp" in cuerpoPred and "strstr" not in cuerpoPred and
        "strncmp" not in cuerpoPred,
        "el predicado compara la linea ENTERA (strcmp): solo se queda la forma exacta "
        "del cable, no cualquier linea que lleve el texto dentro",
        "el predicado usa strstr/strncmp: se quedaria lineas que solo CONTIENEN el "
        "comando -CMD:PIN:1234:LEER_RTC, o una orden mas larga que lo lleve dentro- y "
        "esas ordenes desaparecerian del cable sin llegar al STM32 y sin que nadie las "
        "contestara. Un comando mudo se lee como equipo colgado")

    # =====================================================================
    b.titulo("2. 🔴 Y NINGUNA PUNTA LA ATIENDE: la razon del veto, recalculada")
    # =====================================================================
    # ESTA ES LA COMPROBACION QUE JUSTIFICA EL DISENO, Y LA UNICA QUE NO SE PUEDE
    # SUSTITUIR POR UNA FRASE.
    #
    # despachador.h dice que una linea se queda en el puente SOLO SI ninguna punta tiene
    # un literal para ella. Escrito asi es una afirmacion sobre el codigo, y aqui se
    # mide: se abren los dos bluetooth.cpp y se busca el comando en su cadena de
    # comparaciones -exacta, por prefijo, y en la forma sin PIN-.
    #
    # Que pasa el dia que alguien le escriba una rama a LEER_RTC en el Maestro: esto se
    # pone rojo, y esa es toda la gracia. Entonces habria DOS aparatos contestando una
    # sola orden -el defecto que D-15 cerro el 05/09- y ademas uno de los dos no la
    # recibiria nunca, porque el puente se la queda. La decision se vuelve a tomar con
    # el dato delante en vez de envejecer dentro de un comentario.
    for const, literal in sorted(reclamados.items()):
        orden = literal[4:] if literal.startswith("CMD:") else literal
        atendida_por = []
        for p in PUNTAS:
            codigo = fw.codigo(p, "src", "bluetooth.cpp")
            exactos = set(re.findall(r'strcmp\s*\(\s*accion\s*,\s*"([^"]+)"', codigo))
            prefijos = set(re.findall(r'strncmp\s*\(\s*accion\s*,\s*"([^"]+):"', codigo))
            sinPin = set(re.findall(r'strcmp\s*\(\s*cmd\s*,\s*"CMD:([^"]+)"', codigo))
            if not exactos:
                raise fw.Abortado(
                    "no se leyo ni un strcmp(accion, ...) del bluetooth.cpp del %s. El "
                    "despachador del STM32 es una cadena de comparaciones y si cambio de "
                    "forma, este pack estaria comprobando 'no lo atiende nadie' contra un "
                    "conjunto vacio, que aprueba siempre" % p)
            if orden in (exactos | prefijos | sinPin):
                atendida_por.append(p)
        b.verificar(
            not atendida_por,
            "%s (%s) no la atiende NINGUNA punta: el puente se la puede quedar sin dejar "
            "a nadie sin contestar" % (literal, const),
            "%s la atiende TAMBIEN %s, y el puente se la esta quedando: esa rama del "
            "STM32 no se ejecuta nunca -no le llega la linea- y ademas, si se ejecutara, "
            "habria DOS acuses a una sola orden, que es el defecto que D-15 cerro el "
            "05/09. Hay que decidir de quien es la orden: o se retira la rama del STM32, "
            "o se retira el veto del puente" % (literal, " y ".join(atendida_por)))

    # =====================================================================
    b.titulo("3. El veto se APLICA: puente.cpp pregunta antes de escribir en el cable")
    # =====================================================================
    # Un predicado con la respuesta correcta y sin llamador es N-73 otra vez. Lo que se
    # mide no es "esta en el fichero": es que la escritura hacia el STM32 este DENTRO de
    # la guarda, que es lo unico que impide el reenvio.
    puente = fw.codigo(*PUENTE_CPP)
    ida = _cuerpo(puente, r"static\s+void\s+desdeLaApp\s*\(\s*\)")
    if ida is None:
        raise fw.Abortado(
            "no se hallo desdeLaApp() en %s/src/puente.cpp. Es el sentido de ida y el "
            "unico sitio donde el veto puede aplicarse" % ROL)

    mGuardia = re.search(r"if\s*\(\s*!\s*%s\s*\([^)]*\)\s*\)\s*\{" % re.escape(PREDICADO),
                         ida)
    guardado = _bloque(ida, mGuardia.end() - 1) if mGuardia else None
    b.verificar(
        guardado is not None and "enlace_escribirLinea" in guardado
        and ida.count("enlace_escribirLinea") == guardado.count("enlace_escribirLinea"),
        "la UNICA escritura hacia el STM32 del sentido de ida vive DENTRO de "
        "if (!%s(...)): la linea reclamada no llega al cable" % PREDICADO,
        "hay escrituras hacia el STM32 FUERA de la guarda del predicado (o no hay "
        "guarda). Entonces la consulta viaja igual y las dos puntas contestan "
        "$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO -medido sobre su bluetooth.cpp "
        "compilado-: un rechazo rojo en la app acusando al operario de una clave que no "
        "tecleo, cada vez que pregunta la hora")

    # Y la rama que la atiende tiene que estar DELANTE del reenvio, no detras: si el
    # despachador se llamara despues de haber escrito, el veto no serviria de nada.
    iGuardia = ida.find(PREDICADO)
    iEscribe = ida.find("enlace_escribirLinea")
    b.verificar(
        iGuardia >= 0 and iEscribe >= 0 and iGuardia < iEscribe,
        "se pregunta ANTES de escribir en el cable (guarda=%d, escritura=%d)"
        % (iGuardia, iEscribe),
        "se escribe en el cable antes de preguntar si la linea era del puente. Un veto "
        "que llega despues del envio no veta nada: es enterarse de que la trama ya esta "
        "al otro lado")

    # =====================================================================
    b.titulo("4. Cada motivo del enum tiene rama propia, y ninguna consulta queda muda")
    # =====================================================================
    # LOS MOTIVOS SALEN DEL ENUM, NO DE UNA LISTA TECLEADA. Es el mismo metodo que
    # esp32_03 con ResultadoReloj: tecleando la lista aqui, un motivo nuevo se aprobaria
    # a si mismo -y su consulta se quedaria sin contestar, que se lee como equipo
    # colgado y hace que el tecnico la repita-.
    motivos = _valores_del_enum(cabecera, "MotivoSinHora")
    if len(motivos) < 3:
        raise fw.Abortado(
            "solo se leyeron %d valores del enum MotivoSinHora en %s/include/"
            "reloj_ds3231.h. De ahi sale la lista de 'no se' que la consulta tiene que "
            "saber distinguir; con la lista corta este pack aprobaria un despachador que "
            "contestara lo mismo a todo" % (len(motivos), ROL))

    cuerpoObs = _cuerpo(desp, r"void\s+despachador_observar\s*\([^)]*\)")
    if cuerpoObs is None:
        raise fw.Abortado(
            "no se hallo despachador_observar() en %s/src/despachador.cpp" % ROL)

    sinRama = [v for v in motivos
               if not re.search(r"\bm\s*==\s*%s\b" % re.escape(v), cuerpoObs)]
    b.verificar(
        not sinRama,
        "los %d motivos del enum tienen rama propia en la consulta (%s)"
        % (len(motivos), ", ".join(motivos)),
        "hay motivos SIN RAMA en la consulta: %s. Un motivo sin rama cae al else "
        "generico -o peor, a ninguno- y el tecnico recibe 'no se pudo' donde el firmware "
        "sabia decirle si tiene que repetir la orden, cambiar la pila o cambiar el "
        "modulo" % ", ".join(sinRama))

    # =====================================================================
    b.titulo("5. Cada rama lleva SU literal dentro, y ninguna promete una hora que no leyo")
    # =====================================================================
    # N-89 LITERAL. Si alguien sacara los $ERR a un responderMotivo(m), NINGUNA rama
    # tendria ya un literal, TODAS pasarian por "no promete nada" -los controles
    # negativos incluidos- y este pack seguiria en verde midiendo nada. Por eso se mide
    # DENTRO del bloque de cada rama.
    sinLiteral = []
    for v in motivos:
        m = re.search(r"\bm\s*==\s*%s\b" % re.escape(v), cuerpoObs)
        if m is None:
            continue
        # El bloque de la rama es el que sigue al `)` del if, no el que envuelve a la
        # comparacion -esa vive en la condicion-.
        iLlave = cuerpoObs.find("{", m.end())
        rama = _bloque(cuerpoObs, iLlave) if iLlave >= 0 else None
        if rama is None or '"$ERR' not in rama:
            sinLiteral.append(v)
    b.verificar(
        not sinLiteral,
        "las %d ramas de motivo llevan su propio literal $ERR DENTRO del bloque: un "
        "compositor comun las dejaria a todas sin nada que medir" % len(motivos),
        "estas ramas no tienen un $ERR dentro de su bloque: %s. O contestan por otro "
        "sitio -y entonces este pack no puede ver que contestan- o no contestan, y una "
        "consulta muda se lee como equipo colgado" % ", ".join(sinLiteral))

    # LOS TEXTOS SON DISTINTOS ENTRE SI. Sin esto, la tabla se podria rellenar con el
    # mismo DESC en las siete ramas y el pack saldria verde midiendo nada: es la prueba
    # muerta de N-51 con forma de despachador. Se admite que algun DESC se repita CON EL
    # DE OTRO COMANDO -SIN_RELOJ_NO_RESPONDE es el mismo arreglo en SET_RTC y en
    # LEER_RTC, y dos textos para una averia se leen como dos averias-, pero no entre
    # las ramas de esta misma consulta.
    for const, literal in sorted(reclamados.items()):
        orden = literal[4:] if literal.startswith("CMD:") else literal
        descs = re.findall(r'"\$ERR,[^"]*CMD:%s,DESC:([A-Z0-9_]+)"' % re.escape(orden),
                           desp)
        b.verificar(
            len(descs) >= len(motivos) and len(set(descs)) == len(descs),
            "%s distingue %d motivos de rechazo por texto y los %d son distintos"
            % (orden, len(descs), len(set(descs))),
            "%s tiene %d textos de rechazo para %d motivos del enum, y %d repetidos. Un "
            "'no se pudo' generico no le dice al tecnico si tiene que repetir la orden, "
            "cambiar la pila o cambiar el modulo"
            % (orden, len(descs), len(motivos), len(descs) - len(set(descs))))

    # =====================================================================
    b.titulo("6. La hora solo sale detras de la barrera, y no se inventa nunca")
    # =====================================================================
    # ES N-144 TRAIDO AQUI: aquel dia el equipo se declaro EN HORA con el reloj parado en
    # ceros y publico HORA:00:00:00. Un DS3231 sin pila entrega una fecha PERFECTAMENTE
    # FORMADA y falsa, asi que la unica puerta por la que una hora puede salir es
    # reloj_leer(), que lleva reloj_enHora() delante y no tiene variante "damela igual".
    for const, literal in sorted(reclamados.items()):
        orden = literal[4:] if literal.startswith("CMD:") else literal
        m = re.search(r'"\$ACK,[^"]*CMD:%s,' % re.escape(orden), desp)
        if m is None:
            b.verificar(False, "", "%s no tiene ni un $ACK: una consulta que solo sabe "
                                   "fallar no es una consulta" % orden)
            continue
        # SE MIDE LA GUARDA, NO LA PROXIMIDAD. Lo que hay que comprobar es que el $ACK
        # viva dentro del bloque que SOLO se alcanza si reloj_leer() dijo que si: un
        # "hay una llamada mas arriba en el fichero" aprobaria igual un $ACK colgado del
        # else -o sea, del camino en el que la lectura FALLO-.
        rama = _bloque_que_contiene(desp, m.start())
        antes = desp[:m.start()]
        # La condicion del `if` que abre el bloque que envuelve al $ACK.
        iAbre = antes.rfind("{")
        condicion = antes[max(0, iAbre - 120):iAbre] if iAbre >= 0 else ""
        b.verificar(
            rama is not None and re.search(r"if\s*\(\s*reloj_leer\s*\(", condicion)
            is not None,
            "el $ACK de %s vive DENTRO de if (reloj_leer(...)): solo se alcanza si el "
            "chip entrego la hora por la barrera" % orden,
            "el $ACK de %s no cuelga de un if (reloj_leer(...)): o publica el contenido "
            "de un buffer sin rellenar -con formato perfecto, que es N-144- o cuelga del "
            "camino en el que la lectura fallo" % orden)

    # La hora NO puede componerse de nada que no venga de la estructura que reloj_leer()
    # rellena. Un snprintf con literales de hora seria una hora inventada con formato.
    mSnp = re.search(r'snprintf\([^;]*CMD:LEER_RTC[^;]*;', desp, re.S)
    if mSnp is None:
        mSnp = re.search(r'snprintf\([^;]*"\$ACK,[^;]*;', desp, re.S)
    b.verificar(
        mSnp is not None and re.search(r"leida\.\w+", mSnp.group(0)) is not None,
        "los campos de fecha y hora del $ACK salen de la estructura que reloj_leer() "
        "acaba de rellenar, no de constantes",
        "el $ACK de la consulta compone su fecha con algo que no viene de reloj_leer(). "
        "Una hora con formato perfecto sacada de otro sitio es exactamente la fecha "
        "'bien formada y falsa' que la barrera del DS3231 existe para no publicar")

    # =====================================================================
    b.titulo("7. La consulta NO ESCRIBE: es de solo lectura, y eso se mide")
    # =====================================================================
    # Es la promesa que la app le hace al operario -"no cambia nada: ni el reloj, ni una
    # luz, ni un modo"- y la razon por la que no pide PIN. Si esta rama llamara a
    # reloj_ajustar(), esa promesa seria falsa y encima el comando entraria sin clave.
    mRama = re.search(r"if\s*\(\s*%s\s*\(\s*linea\s*\)\s*\)\s*\{" % re.escape(PREDICADO),
                      cuerpoObs)
    ramaConsulta = _bloque(cuerpoObs, mRama.end() - 1) if mRama else None
    if ramaConsulta is None:
        raise fw.Abortado(
            "no se hallo la rama que atiende lo reclamado -if (%s(linea)) { ... }- en "
            "despachador_observar(). Sin ella no se puede comprobar que la consulta no "
            "escriba" % PREDICADO)
    b.verificar(
        "reloj_ajustar" not in ramaConsulta,
        "la rama de la consulta NO llama a reloj_ajustar(): es de solo lectura, que es "
        "lo que la app le promete al operario y lo que justifica que entre sin PIN",
        "la rama de la consulta llama a reloj_ajustar(). Entonces NO es una consulta: "
        "cambia el reloj de un equipo que esta en la calle, la app miente al decir que "
        "no cambia nada, y encima lo hace sin PIN porque se registro como lectura")

    # =====================================================================
    b.titulo("CONTROLES NEGATIVOS")
    # =====================================================================
    # Contra bloques sinteticos con las MISMAS funciones reales. Si el detector aprobara
    # estos, todos los OK de arriba serian decoracion.
    falso = ('bool p(const char* l){ return strstr(l, "CMD:LEER_RTC") != NULL; }')
    cuerpoFalso = _cuerpo(falso, r"bool\s+p\s*\([^)]*\)")
    b.control_negativo(
        cuerpoFalso is not None and "strstr" in cuerpoFalso,
        "un predicado escrito con strstr se detecta: se quedaria lineas que solo "
        "CONTIENEN el comando, y esas desaparecerian del cable sin contestar")

    escribe = ('{ if (p(linea)) { FechaHora f; reloj_ajustar(&f); '
               'emitir("$ACK,CMD:LEER_RTC,RESULT:OK"); } }')
    mE = re.search(r"if\s*\(\s*p\s*\(\s*linea\s*\)\s*\)\s*\{", escribe)
    b.control_negativo(
        "reloj_ajustar" in _bloque(escribe, mE.end() - 1),
        "una consulta que ESCRIBE el reloj se detecta: mide el bloque de la rama, no el "
        "nombre del comando")

    inventada = 'snprintf(p, sizeof(p), "$ACK,CMD:LEER_RTC,RESULT:OK,HORA:00:00:00");'
    b.control_negativo(
        re.search(r"leida\.\w+", inventada) is None,
        "un $ACK que compone la hora con literales -y no con lo que reloj_leer() "
        "devolvio- se detecta: es N-144, el equipo declarandose en hora con el reloj "
        "parado en ceros")

    fuera = ('{ if (c==term) { bool prop = (enlace_escribirLinea(x, n) > 0); '
             'if (!despachador_esParaElPuente(x)) { } observar(x, prop); } }')
    mF = re.search(r"if\s*\(\s*!\s*despachador_esParaElPuente\s*\([^)]*\)\s*\)\s*\{", fuera)
    b.control_negativo(
        mF is not None and "enlace_escribirLinea" not in _bloque(fuera, mF.end() - 1),
        "una guarda VACIA con la escritura fuera de ella se detecta: el veto tiene que "
        "envolver la escritura, no acompanarla")
