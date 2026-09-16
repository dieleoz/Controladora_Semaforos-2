# ===== banco/packs/version_01_sello_de_firmware.py =====
#
# LA PREGUNTA ABIERTA QUE ESTE PACK CONTESTA (CLAUDE.md 16):
#
#   "¿Puede el equipo decir que firmware lleva dentro, y puede MENTIR al decirlo?"
#
# Estaba abierta y medida el 16/09: grep de VERSION / FW_VER / GIT_HASH / __DATE__ sobre
# Maestro/{src,include} daba CERO. CLAUDE.md 0.2 exige que una foto de campo traiga el hash
# de lo que habia dentro, y ese hash vivia SOLO en la memoria de quien cargo la tarjeta -la
# cinta del Sisga del 16/09 se pudo atribuir porque el responsable se acordaba-. Este pack
# no vuelve a certificar nada ya certificado: mide una propiedad de vida que nadie ejercia.
#
# Y LO QUE VIGILA NO ES QUE EL SELLO EXISTA -eso lo ve cualquiera- SINO QUE NO PUEDA
# MENTIR, que son cuatro mentiras distintas y cada una tiene su comprobacion:
#
#   1. DECLARAR UN COMMIT QUE EL BINARIO NO CONTIENE. Un binario compilado sobre trabajo
#      sin comitear lleva ese commit MAS lo que hubiera encima, que no esta en ningun
#      sitio. Sin la marca de sucio, la foto de campo se atribuye al commit equivocado: el
#      defecto que el sello viene a cerrar, con una capa de autoridad encima.  (A-3, A-4)
#   2. CONTESTAR "OK" SIN SABERLO. CLAUDE.md 2: un acuse que no depende de lo que se pudo
#      componer es una mentira con formato de exito. Un binario sin sellar NO SABE que
#      firmware lleva y tiene que decirlo con un $ERR, no con RESULT:OK.            (A-6)
#   3. UN SELLO ESCRITO A MANO. CLAUDE.md 14: un numero en un sitio que no puede
#      recalcularlo nace caducado, y este ademas miente sobre de que commit sale el
#      binario, que es lo unico que afirma.                                        (A-2)
#   4. MORIR EN SILENCIO EN EL FICHERO DE CONFIGURACION. Esta no se ve leyendo, y es la
#      razon principal de que esto sea un pack y no un comentario: ver A-3.
#
# ---------------------------------------------------------------------------------
# EL BORDE, ESCRITO Y JUSTIFICADO (CLAUDE.md 7). LEASE ANTES QUE EL VERDE.
# ---------------------------------------------------------------------------------
#
# 1. ES UN PACK DE TEXTO Y NO CERTIFICA COMPORTAMIENTO (CLAUDE.md 6.3). Un verde aqui NO
#    dice que el equipo contestara la version en una tarjeta: dice que el sello esta
#    inyectado, que las dos respuestas existen en el fuente, que la que no sabe no dice OK
#    y que ninguna se puede truncar. Que el literal acaba DENTRO del binario se midio
#    aparte, el 16/09, con `strings` sobre los tres .elf.
#
# 2. EL SELLO NO TIENE DOS RESPUESTAS EN UN MISMO BINARIO, Y ESO ES LO CORRECTO. FW_SELLADO
#    se decide en el preprocesador porque es un hecho del tiempo de compilacion: un
#    `if (FW_SELLADO)` compilaria a la guarda de un solo valor que CLAUDE.md 6.2 persigue
#    -el enum que sale como `movs r0,#1`-. La consecuencia para el instrumento es que
#    NINGUN arnes en ejecucion puede ver las dos ramas, asi que quien tiene que verlas es
#    este pack: lee los DOS literales del fuente y exige cada uno en su rama (A-6). Las
#    tres salidas se midieron ademas a mano el 16/09 con `g++ -E`: con sello, sin sello, y
#    con el hash puesto pero sin la marca de suciedad -que tiene que salir SIN SELLAR-.
#
# 3. LO QUE ESTE PACK NO PUEDE COMPROBAR, DICHO PARA QUE NO SE LEA DE MAS: que el hash que
#    entra sea el del arbol de verdad. Eso lo contesta git. Lo que si se exige es que la
#    pregunta se le haga a git y no a una constante escrita a mano (A-2).
#
# NO LLEVA ETIQUETA SFTY, por el mismo motivo que esp32_01, esp32_02 y esp32_10: asignar
# numero nuevo es del responsable (AB-8), y una fila de trazabilidad que no corresponde a
# ninguna regla es peor que una fila vacia -la vacia no miente-.

import configparser
import re

NOMBRE = "version_01_sello_de_firmware"
DESCRIPCION = "las tres puntas dicen que firmware llevan, y no pueden decirlo mal"

# Las rutas van como TRIPLES LITERALES y con el rol escrito, no por variable: la guarda de
# rutas de compuerta.py las lee POR TEXTO. Con una variable delante solo ve el par
# ("include", "version_fw.h") -que no dice el rol- y entonces lo exige en LAS DOS puntas
# STM32, que es como el 01/09 ABORTO la guarda entera reclamando ficheros del ESP32 bajo
# Maestro/. Un ABORTADO en la guarda apaga la compuerta completa.
SELLO_M = ("Maestro", "include", "version_fw.h")
SELLO_E = ("Esclavo", "include", "version_fw.h")
SELLO_X = ("ESP32_Expansion", "include", "version_fw.h")
BT_M = ("Maestro", "src", "bluetooth.cpp")
BT_E = ("Esclavo", "src", "bluetooth.cpp")
VIGILANTE = ("ESP32_Expansion", "src", "vigilante.cpp")
DESPACHADOR = ("ESP32_Expansion", "src", "despachador.cpp")
TRAMA = ("ESP32_Expansion", "src", "trama.cpp")
CONTRATO = ("ESP32_Expansion", "include", "contrato.h")

# Los tres platformio.ini NO viven bajo src/ ni include/, asi que no son un triple que la
# guarda de rutas pueda censar. fw.texto() ABORTA si falta cualquiera, que es la misma
# proteccion por otra puerta.
INI = (("Maestro", "platformio.ini"),
       ("Esclavo", "platformio.ini"),
       ("ESP32_Expansion", "platformio.ini"))

# EL NODO DE CADA PUNTA, ESCRITO A MANO Y NO LEIDO DEL FUENTE. Es el molde de esp32_05:
# "si el pack leyera los nombres del propio fuente, un nodo nuevo se aprobaria a si mismo".
# Aqui muerde de verdad, porque el nodo es lo que permite atribuir un sello a un poste: un
# acuse con el nodo equivocado manda a diagnosticar el otro extremo del cruce.
NODOS = {"Maestro": "NODE:MAESTRO", "Esclavo": "NODE:ESCLAVO"}

# Bytes que la trama usa como ESTRUCTURA: la coma separa campos, el '*' abre el checksum y
# el '$' abre la trama. Cualquiera de ellos dentro de una marca del sello partiria la trama
# por dentro con el checksum casando igual, porque se calcula sobre la linea entera. Es la
# misma lista que esp32_10 exige a los nombres de causa, y por el mismo motivo.
PROHIBIDOS = (",", "*", "$", "\r", "\n")

# Los prefijos de comentario EN LINEA con que platformio lee su propio .ini. MEDIDO, no
# supuesto: platformio/project/config.py:97 construye
#     configparser.ConfigParser(inline_comment_prefixes=("#", ";"))
# y ese es el borde que A-3 vigila. Se escriben aqui, y no se leen de platformio, por la
# razon de esp32_10: "un instrumento no puede depender del entorno de quien lo llama"; un
# pack que abriera el paquete de platformio abortaria en la maquina que no lo tenga y
# dejaria abierta por ABORTADO justo la puerta que vino a cerrar.
PREFIJOS_EN_LINEA = ("#", ";")


def _bloques_sellado(codigo):
    """Los pares (rama con sello, rama sin sello) de cada #if FW_SELLADO del fichero.

    Sin anidamiento a proposito: un #if dentro de otro haria que este lector cerrara en el
    #endif equivocado y midiera media rama. Si alguien anida, los literales que A-6 exige
    no aparecen donde toca y el pack lo dice en vez de aprobar a medias."""
    return re.findall(r"#if\s+FW_SELLADO\b(.*?)#else\b(.*?)#endif", codigo, re.S)


def _literal(texto, patron, que, fw, donde):
    """Un literal de cadena del C++. ABORTA si no aparece.

    Misma regla que fw.constante() con los numeros -sin valor por defecto, nunca-: una cota
    calculada sobre una cadena inventada sale en verde."""
    m = re.search(patron, texto)
    if not m:
        raise fw.Abortado(
            "no se pudo leer de %s %s (patron %r). Sin ese literal la cota del sello se "
            "calcularia sobre otra cosa que el firmware y seguiria dando PASS"
            % (donde, que, patron))
    return m.group(1)


def _expandir(cadena):
    """Los escapes de C que caben en un literal de formato, a su byte real."""
    return cadena.replace("\\r", "\r").replace("\\n", "\n").replace("\\t", "\t")


def _valor_ini(texto, clave):
    """El valor de una clave del .ini LEIDO COMO LO LEE PLATFORMIO.

    No se busca la linea con un regex -eso seria leerla-: se pasa por el mismo ConfigParser
    con los mismos inline_comment_prefixes. Es la unica forma de que A-3 sea una MEDIDA del
    borde, que es justo lo que no se ve leyendo."""
    cp = configparser.ConfigParser(inline_comment_prefixes=PREFIJOS_EN_LINEA)
    cp.read_string(texto)
    for seccion in cp.sections():
        if cp.has_option(seccion, clave):
            return cp.get(seccion, clave)
    return None


def _orden_de_sellado(valor):
    """Las lineas de build_flags que son una orden de sellado, como las ve platformio."""
    if valor is None:
        return []
    return [x.strip() for x in valor.splitlines() if x.strip().startswith("!")]


def _ancho_decl(codigo, nombre):
    """El ancho de un buffer declarado, leido del C++.

    SE LEE, NO SE TOCA: esp32_07 y esp32_09 direccionan esta misma declaracion por TEXTO
    -"char tramaCompleta[(\\d+)]"-, asi que ponerle un nombre a ese numero para poder
    escribir un static_assert los ABORTARIA a los dos (CLAUDE.md 5). Por eso la cota del
    sello vive aqui y no en un static_assert del firmware."""
    m = re.search(r"char\s+%s\s*\[\s*(\d+)\s*\]" % re.escape(nombre), codigo)
    return None if not m else int(m.group(1))


def correr(b, fw):
    b.titulo("El sello de firmware: las tres puntas dicen que llevan, y no pueden mentir")

    sellos = (SELLO_M, SELLO_E, SELLO_X)
    huellas = {t[0]: fw.huella(*t) for t in sellos}
    sello = fw.texto(*SELLO_M)

    # ---- A-1: UN SOLO SELLO, EL MISMO BYTE A BYTE EN LAS TRES PUNTAS -----------
    #
    # Por el fichero ENTERO y no por una constante, con el molde de costura_01: la igualdad
    # tiene que romperse por cualquier byte que cambie, no solo por los que alguien penso
    # en vigilar. Dos sellos con formato distinto son dos convenciones que el tecnico tiene
    # que aprender en el mismo poste, y un acta que compare un sello contra un commit deja
    # de poder hacerse a maquina.
    b.verificar(
        len(set(huellas.values())) == 1,
        "A-1: version_fw.h es IDENTICO en las tres puntas (sha256 %s...)"
        % sorted(set(huellas.values()))[0][:12],
        "version_fw.h DIVERGE entre puntas: %s. Un sello con formato distinto por punta "
        "obliga al tecnico a aprender dos convenciones en el mismo poste, y un acta que "
        "compare un sello contra un commit deja de poder hacerse a maquina"
        % {k: v[:12] for k, v in huellas.items()})

    # ---- A-2: EL SELLO SE INYECTA, NO SE ESCRIBE -------------------------------
    a_mano = ["/".join(t) for t in sellos + (BT_M, BT_E, VIGILANTE)
              if re.search(r"#\s*define\s+FW_HASH\b", fw.texto(*t))]
    b.verificar(
        not a_mano,
        "A-2: ningun fuente define FW_HASH: el sello entra solo por la linea de "
        "platformio.ini, que es la unica que puede preguntarselo a git",
        "hay fuentes que definen FW_HASH a mano: %s. Un sello escrito a mano nace caducado "
        "(CLAUDE.md 14) y ademas MIENTE con autoridad, porque lo que afirma es de que "
        "commit sale este binario -y eso es todo lo que afirma-" % a_mano)

    # ---- A-3: LA LINEA DEL SELLO NO PUEDE MORIR EN SILENCIO EN EL .ini ---------
    #
    # 🔴 ESTA ES LA RAZON PRINCIPAL DE QUE ESTO SEA UN PACK.
    #
    # platformio lee su .ini con ConfigParser(inline_comment_prefixes=("#", ";")), que CORTA
    # el valor en el primer '#' o ';' precedido de espacio. La orden de sellado es un
    # one-liner de python lleno de ';', asi que UN SOLO ESPACIO de mas la deja a medias:
    # python falla, platformio sigue compilando, el binario sale SIN SELLAR y el unico
    # sintoma es que el equipo contesta que no sabe que firmware lleva. Nadie relacionaria
    # eso con un espacio en un fichero de configuracion, y el binario ya estaria en un
    # poste.
    ordenes = {}
    for t in INI:
        valor = _valor_ini(fw.texto(*t), "build_flags")
        if valor is None:
            raise fw.Abortado(
                "no se pudo leer build_flags de %s/platformio.ini con el mismo "
                "ConfigParser que usa platformio. Sin leerlo como lo lee el compilador, "
                "aprobar el sello seria aprobarlo sin mirar" % t[0])
        vistas = _orden_de_sellado(valor)
        b.verificar(
            len(vistas) == 1,
            "A-3: %s: platformio ve UNA sola orden de sellado en build_flags" % t[0],
            "A-3: %s: platformio ve %d ordenes de sellado y tiene que ver una. Si ve CERO "
            "el binario sale sin sellar; si ve dos, una pisa a la otra y nadie sabe cual"
            % (t[0], len(vistas)))
        if len(vistas) != 1:
            raise fw.Abortado(
                "%s/platformio.ini no deja UNA orden de sellado legible: las cotas de "
                "abajo se calcularian sobre un sello que no existe" % t[0])

        # Que lo que ConfigParser devuelve sea el comando COMPLETO. Si un ';' o un '#'
        # llevara espacio delante, aqui llegaria cortado justo por el final -que es donde
        # se imprimen los dos -D-.
        crudo = None
        for linea in fw.texto(*t).splitlines():
            if linea.strip().startswith("!") and "FW_HASH" in linea:
                crudo = linea.strip()
        b.verificar(
            crudo is not None and crudo == vistas[0],
            "A-3: %s: la orden de sellado sobrevive ENTERA al lector de platformio (%d "
            "caracteres: ni un ';' ni un '#' con espacio delante)" % (t[0], len(vistas[0])),
            "A-3: %s: platformio RECORTA la orden de sellado. Su ConfigParser corta el "
            "valor en un '#' o un ';' precedido de espacio, y lo que se pierde es el final "
            "del comando, donde se imprimen los dos -D. El binario saldria SIN SELLAR y el "
            "unico sintoma seria que el equipo dice que no lo sabe.\n"
            "      en el fichero: %r\n      lo que ve pio: %r" % (t[0], crudo, vistas[0]))
        ordenes[t[0]] = vistas[0]

    # La MISMA orden en las tres: tres sellos compuestos por tres comandos distintos son
    # tres formatos que nadie compara, y el dia que uno cambie el cruce publicaria dos
    # sellos que no se pueden restar.
    b.verificar(
        len(set(ordenes.values())) == 1,
        "A-3: la orden de sellado es IDENTICA en los tres platformio.ini",
        "la orden de sellado DIFIERE entre puntas: %s. Tres comandos distintos son tres "
        "formatos de sello que nadie compara" % {k: v[:70] for k, v in ordenes.items()})

    orden = ordenes["Maestro"]

    # Y pregunta las DOS cosas: el commit y el estado del arbol. Sin el censo de suciedad,
    # el sello declararia limpio un binario que nadie midio.
    b.verificar(
        "rev-parse" in orden and "status" in orden and "porcelain" in orden,
        "A-3: la orden pregunta las DOS cosas a git: el commit (rev-parse) y el estado del "
        "arbol (status --porcelain, que cuenta tambien lo no seguido)",
        "la orden de sellado NO pregunta las dos cosas: %r. Un sello que no mira el arbol "
        "declara limpio un binario compilado sobre trabajo sin comitear, que es la unica "
        "mentira que este sello no puede permitirse" % orden)

    # EL ANCHO DEL HASH SALE DE LA ORDEN, NO DE UN 7 ESCRITO AQUI. De el cuelga la cota de
    # A-8: el dia que alguien ponga --short=12 o quite el --short -y git da 40-, la
    # desigualdad cambia sola en vez de seguir aprobando la de ayer (N-71).
    m = re.search(r"--short=(\d+)", orden)
    if not m:
        raise fw.Abortado(
            "la orden de sellado no fija el ancho del hash (--short=N). Sin ese numero la "
            "cota del sello se calcularia sobre un ancho supuesto, y un hash completo de "
            "git son 40 caracteres: el error seria de 33 bytes por trama")
    digitos = int(m.group(1))

    # ---- A-4: EL SELLO EXIGE LAS DOS MACROS -----------------------------------
    #
    # Es la simetrica de CLAUDE.md 6.2: antes de fiarse de una bandera se mira que pasa si
    # la OTRA no llega. Con FW_HASH puesto y FW_SUCIO ausente, un `#ifdef FW_HASH` daria el
    # sello por bueno y publicaria un arbol limpio que NADIE MIDIO.
    b.verificar(
        re.search(r"#if\s+defined\s*\(\s*FW_HASH\s*\)\s*&&\s*"
                  r"defined\s*\(\s*FW_SUCIO\s*\)", sello) is not None,
        "A-4: FW_SELLADO exige LAS DOS macros: con el hash puesto y la suciedad ausente el "
        "firmware dice que no lo sabe, en vez de declarar un arbol limpio sin medir",
        "FW_SELLADO no exige las dos macros. Con solo el hash, un binario compilado sobre "
        "trabajo sin comitear declararia el commit LIMPIO: el sello estaria certificando "
        "lo contrario de lo que existe para decir")

    # ---- A-5: LAS DOS MARCAS, Y NINGUNA PARTE LA TRAMA -----------------------
    marca = _literal(sello, r'#\s*define\s+FW_MARCA_SUCIO\s+"([^"]*)"',
                     "la marca de arbol sucio", fw, "version_fw.h")
    sin_sello = _literal(sello, r'#\s*define\s+FW_SIN_SELLO\s+"([^"]*)"',
                         "la marca de sello ausente", fw, "version_fw.h")

    sucias = [x for x in (marca, sin_sello) if any(p in x for p in PROHIBIDOS)]
    b.verificar(
        not sucias,
        "A-5: las dos marcas del sello (%r sucio, %r sin sellar) no llevan coma, asterisco "
        "ni '$': no pueden partir la trama por dentro" % (marca, sin_sello),
        "hay marcas del sello con bytes que la trama usa como estructura: %s. La coma "
        "separa campos y el '*' abre el checksum: el parser de la app leeria un campo que "
        "nadie escribio, y el checksum seguiria casando porque se calcula sobre la linea "
        "entera" % sucias)

    b.verificar(
        len(marca) >= 2 and marca != sin_sello,
        "A-5: la marca de sucio (%r) es propia y distinta de la de 'todavia no lo se' "
        "(%r): son dos diagnosticos y no se pueden leer igual" % (marca, sin_sello),
        "la marca de arbol sucio (%r) no se distingue de la de sello ausente (%r). "
        "'no se que firmware llevo' y 'llevo este commit MAS lo que no esta en ningun "
        "sitio' mandan a hacer dos cosas distintas" % (marca, sin_sello))

    # El ancho maximo que el campo FW: puede llegar a medir. De aqui sale A-8, y sale del
    # fuente entero: el ancho del hash de la orden del .ini y la marca de version_fw.h.
    fw_max = digitos + len(marca)

    # ---- A-6: LAS DOS RESPUESTAS, CADA UNA CON SU LITERAL DENTRO DE SU RAMA ----
    #
    # CLAUDE.md 2: un acuse que no depende de lo que la llamada devolvio es una mentira con
    # formato de exito. Aqui "lo que la llamada devolvio" es si el sello entro: un binario
    # sin sellar NO SABE que firmware lleva, y decirlo con RESULT:OK seria exactamente eso.
    #
    # Y los literales se exigen DENTRO de cada rama (N-89). Un responderVersion(sellado)
    # que armara la trama en otro sitio dejaria las dos ramas sin literal, las dos pasarian
    # por "no promete nada" y este pack seguiria verde midiendo nada.
    peores = {}
    for t in (BT_M, BT_E):
        punta = t[0]
        codigo = fw.codigo(*t)
        bloques = _bloques_sellado(codigo)
        if not bloques:
            raise fw.Abortado(
                "%s/src/bluetooth.cpp no tiene un solo bloque #if FW_SELLADO: sin el no "
                "hay respuesta que medir, y aprobar aqui seria aprobar el hueco" % punta)
        b.verificar(
            True,
            "A-6: %s: %d bloque(s) #if FW_SELLADO en el despachador" % (punta, len(bloques)),
            "no deberia llegarse aqui")

        for i, (con, sin) in enumerate(bloques, 1):
            b.verificar(
                "$ACK,CMD:VERSION,RESULT:OK" in con and "$ERR" not in con,
                "A-6: %s bloque %d: con sello contesta $ACK,CMD:VERSION,RESULT:OK y solo "
                "eso" % (punta, i),
                "A-6: %s bloque %d: la rama CON sello no contesta un $ACK,CMD:VERSION,"
                "RESULT:OK limpio. Un comando que no contesta se lee como equipo colgado y "
                "el tecnico lo repite; uno que contesta dos cosas deja al operario "
                "eligiendo cual se cree" % (punta, i))
            b.verificar(
                "$ERR,CMD:VERSION,DESC:SIN_SELLAR" in sin and "RESULT:OK" not in sin,
                "A-6: %s bloque %d: SIN sello contesta $ERR,...,DESC:SIN_SELLAR y NUNCA "
                "RESULT:OK (CLAUDE.md 2)" % (punta, i),
                "A-6: %s bloque %d: un binario sin sellar contesta OK, o no contesta. "
                "CLAUDE.md 2: el acuse tiene que depender de lo que se pudo componer, y "
                "aqui lo que no se pudo componer es LA RESPUESTA ENTERA -el equipo no sabe "
                "que firmware lleva-. Es el $ACK de SET_RTC otra vez: el tecnico se va del "
                "poste creyendo que anoto la version" % (punta, i))
            b.verificar(
                NODOS[punta] in con and NODOS[punta] in sin,
                "A-6: %s bloque %d: las dos respuestas llevan %s, asi que la linea se "
                "puede pegar SOLA en un acta y seguir diciendo de que poste sale"
                % (punta, i, NODOS[punta]),
                "A-6: %s bloque %d: la respuesta del sello no lleva %s. En los demas "
                "acuses el nodo lo dice el $STATUS que va al lado, pero esta trama esta "
                "hecha para VIAJAR SOLA -pegada en un acta o en un mensaje-: un sello sin "
                "nodo es la atribucion de CLAUDE.md 0.2 sin resolver, movida un metro"
                % (punta, i, NODOS[punta]))

        # Los prefijos literales de las dos respuestas, LEIDOS DEL C++. De ellos sale A-8.
        pre_ok = re.findall(r'enviarTramaConCrc\s*\(\s*"([^"]*)"\s*FW_TEXTO\s*\)', codigo)
        pre_no = re.findall(r'enviarTramaConCrc\s*\(\s*"([^"]*)"\s*FW_SIN_SELLO\s*\)',
                            codigo)
        if not pre_ok or not pre_no:
            raise fw.Abortado(
                "no se hallaron en %s/src/bluetooth.cpp las dos respuestas del sello en la "
                'forma "...FW:" FW_TEXTO / FW_SIN_SELLO. Son de donde sale la cota, y '
                "calcularla sobre una cadena inventada saldria en verde" % punta)
        peores[punta] = max(max(len(p) for p in pre_ok) + fw_max,
                            max(len(p) for p in pre_no) + len(sin_sello))

        # Y la orden entra por las DOS puertas -con PIN y sin PIN-. Aceptar solo una la
        # hace depender de en que lista la ponga la app, que no vive en este arbol; y la
        # version es lo primero que se pregunta cuando algo va mal: un AUTH_FAILED ahi
        # manda a buscar una clave en vez de a mirar el firmware.
        con_pin = re.search(r'strcmp\s*\(\s*accion\s*,\s*"VERSION"\s*\)', codigo)
        sin_pin = (re.search(r'strcmp\s*\(\s*cmd\s*,\s*"CMD:VERSION"\s*\)', codigo)
                   or re.search(r'strcmp\s*\(\s*cmd\s*\+\s*4\s*,\s*"VERSION"\s*\)', codigo))
        b.verificar(
            con_pin is not None and sin_pin is not None,
            "A-6: %s: la version se acepta por las DOS puertas, con PIN y sin PIN" % punta,
            "A-6: %s: la version entra por una sola puerta (con PIN: %s / sin PIN: %s). "
            "Cual usa el telefono lo decide la lista SIN_PIN de la app, que no vive en "
            "este arbol: con una sola puerta el equipo contesta AUTH_FAILED a la primera "
            "pregunta que se hace cuando algo va mal"
            % (punta, con_pin is not None, sin_pin is not None))

    # ---- A-7: EL SELLO NO VA EN EL $STATUS -----------------------------------
    #
    # El $STATUS de las dos puntas es EL BORDE DEL CABLE de este proyecto: su payload esta
    # en 155 B y lo dice su propio fuente. Un campo que NO CAMBIA NUNCA mientras el equipo
    # esta encendido, publicado cada 2 s, le come el margen a los que si cambian; y cuando
    # uno de verdad no cabe, la trama se trunca POR EL FINAL y el checksum sale calculado
    # sobre el trozo (N-108).
    for t in (BT_M, BT_E):
        punta = t[0]
        formatos = re.findall(r'"(\$STATUS[^"]*)"', fw.codigo(*t))
        if not formatos:
            raise fw.Abortado(
                "no se hallo el formato del $STATUS en %s/src/bluetooth.cpp: sin el no se "
                "puede comprobar que el sello NO se cuela en la trama del borde" % punta)
        con_fw = [f for f in formatos if "FW:" in f]
        b.verificar(
            not con_fw,
            "A-7: %s: el sello NO va en el $STATUS (%d formato(s) revisado(s))"
            % (punta, len(formatos)),
            "A-7: %s: el sello se ha metido en el $STATUS: %s. Ese payload es el borde del "
            "cable y sale cada 2 s: un campo constante ahi le quita el margen a los que "
            "cambian, y cuando uno no cabe la trama se trunca por el final con el checksum "
            "calculado sobre el trozo (N-108)" % (punta, con_fw))

    # ---- A-8: LA COTA. NINGUNA RESPUESTA SE PUEDE TRUNCAR --------------------
    #
    # No hay snprintf en la respuesta -es un literal concatenado por el preprocesador-, asi
    # que lo unico que puede cortarla es el envoltorio: enviarTramaConCrc() mete el payload
    # en su tramaCompleta[] junto al "*XX\r\n". Los tres sumandos se releen del C++ en cada
    # corrida: el prefijo de la respuesta, el ancho del hash (de la orden del .ini), y el
    # formato del cierre. Ni uno se teclea aqui.
    #
    # POR QUE ESTA COTA VIVE EN EL PACK Y NO EN UN static_assert: para escribirlo haria
    # falta un NOMBRE para el 160 de tramaCompleta[], y esp32_07 y esp32_09 leen esa
    # declaracion por TEXTO -"char tramaCompleta[(\d+)]"-. Nombrarla los aborta a los dos
    # (CLAUDE.md 5), y un ABORTADO es una puerta abierta.
    cierre = _expandir(_literal(
        fw.codigo(*BT_M),
        r'snprintf\s*\(\s*tramaCompleta\s*,[^,]+,\s*"((?:[^"\\]|\\.)*)"',
        "el formato con que enviarTramaConCrc() cierra la trama", fw,
        "Maestro/src/bluetooth.cpp"))
    sobrecoste = len(re.sub(r"%02X", "XX", cierre).replace("%s", ""))

    for t in (BT_M, BT_E):
        punta = t[0]
        cap = _ancho_decl(fw.codigo(*t), "tramaCompleta")
        if cap is None:
            raise fw.Abortado(
                "no se hallo tramaCompleta[] en %s/src/bluetooth.cpp: es el techo de toda "
                "trama que esta punta pone en el cable, y sin el la cota del sello se "
                "calcularia contra nada" % punta)
        total = peores[punta] + sobrecoste + 1   # +1 del nulo
        b.verificar(
            total <= cap,
            "A-8: %s: la respuesta del sello no se puede truncar: %d de payload + %d de "
            "cierre + 1 = %d <= tramaCompleta[%d]"
            % (punta, peores[punta], sobrecoste, total, cap),
            "A-8: %s: LA RESPUESTA DEL SELLO SE TRUNCA: %d bytes contra un "
            "tramaCompleta[%d]. snprintf corta EN SILENCIO y el checksum sale calculado "
            "sobre el trozo: la app lo descarta y desde el poste se lee como 'el equipo no "
            "contesta la version'. Se arregla acortando el campo, nunca agrandando el "
            "buffer (CLAUDE.md 10)" % (punta, total, cap))

    # ---- A-9: EL PUENTE LO DICE POR SU PARTE, Y NO LE TAPA LA BOCA AL EQUIPO --
    #
    # Si el puente RECLAMARA la linea de la version, despachador_esParaElPuente() se la
    # quedaria entera y la orden no cruzaria: el que no podria contestar seria EL EQUIPO,
    # que es de quien se pregunta la version en CLAUDE.md 0.2. El accesorio no puede
    # taparle la boca al controlador para hablar el.
    desp = fw.codigo(*DESPACHADOR)
    b.verificar(
        "VERSION" not in desp,
        "A-9: el despachador del puente NO reclama la linea de la version: cruza al STM32 "
        "y contesta el equipo, que es de quien se pregunta",
        "A-9: el despachador del ESP32 nombra VERSION. Si la reclama, esa linea NO CRUZA "
        "-desde D-20 lo reclamado se queda aqui- y el CONTROLADOR se queda sin poder decir "
        "que firmware lleva, que es justo lo que 1.51 existe para arreglar")

    vig = fw.codigo(*VIGILANTE)
    pre_puente = re.findall(r'puente_emitirPropio\s*\(\s*"([^"]*)"\s*FW_TEXTO\s*\)', vig)
    b.verificar(
        len(pre_puente) == 1,
        "A-9: el puente emite su sello por puente_emitirPropio() -hacia la app y solo "
        "hacia la app-, una vez y con su literal a la vista",
        "A-9: el sello del puente no sale por puente_emitirPropio() en la forma "
        '"...FW:" FW_TEXTO (%d hallazgo(s)). Por enlace_escribirLinea() iria hacia el '
        "STM32, que es trafico que el accesorio no puede originar (B-1), y sacado a un "
        "compositor dejaria la rama sin literal y este pack midiendo nada (N-89)"
        % len(pre_puente))

    if pre_puente:
        b.verificar(
            "NODE:PUENTE" in pre_puente[0],
            "A-9: el sello del puente va marcado NODE:PUENTE: no se puede confundir con el "
            "del controlador, que se carga por otro camino y puede ser otro",
            "A-9: el sello del puente NO lleva NODE:PUENTE. Un $EVENT del accesorio que "
            "parezca del STM32 manda a diagnosticar el poste equivocado, y aqui ademas "
            "haria creer que el controlador lleva un firmware que no lleva")

        # SALE AUNQUE EL PARTE NO QUEPA. Son dos hechos independientes, y el caso en que el
        # parte se cae es precisamente el que mas falta hace poder atribuir a un commit.
        cuerpo = vig[vig.find("void vigilante_declarar"):]
        i_sello = cuerpo.find(pre_puente[0])
        i_return = cuerpo.find("if (n == 0) return;")
        b.verificar(
            i_sello >= 0 and i_return >= 0 and i_sello < i_return,
            "A-9: el sello sale ANTES del return que abandona un parte que no cabe: si el "
            "parte se cae, el sello sigue saliendo",
            "A-9: el sello del puente sale DESPUES del return que abandona un parte que no "
            "cabe (sello en %d, return en %d). Son dos hechos independientes, y el caso en "
            "que el parte no cabe es justo el que mas falta hace poder atribuir a un "
            "commit" % (i_sello, i_return))

        # Y la linea entera cabe en el buffer de salida hacia la app, con el cierre que
        # pone trama_componer(). Aqui el truncado no se ve: trama_componer() devuelve 0 y
        # el sello no sale NUNCA, en silencio.
        cierre_x = _expandir(_literal(
            fw.codigo(*TRAMA),
            r'snprintf\s*\(\s*destino\s*,\s*capacidad\s*,\s*"((?:[^"\\]|\\.)*)"',
            "el formato con que trama_componer() cierra la trama", fw,
            "ESP32_Expansion/src/trama.cpp"))
        sobre_x = len(re.sub(r"%02X", "XX", cierre_x).replace("%s", ""))
        salida = fw.constante(CONTRATO, r"#define\s+BUF_SALIDA_APP\s+(\d+)",
                              "el buffer de salida hacia la app")
        total_x = len(pre_puente[0]) + fw_max + sobre_x + 1
        b.verificar(
            total_x <= salida,
            "A-9: el sello del puente cabe en el buffer de salida: %d + %d de cierre + 1 = "
            "%d <= BUF_SALIDA_APP (%d)"
            % (len(pre_puente[0]) + fw_max, sobre_x, total_x, salida),
            "A-9: EL SELLO DEL PUENTE NO CABE EN BUF_SALIDA_APP: %d contra %d. "
            "trama_componer() devolveria 0 y el sello no saldria NUNCA, en silencio, que "
            "es el reinicio mudo de esp32_10 con otro sujeto" % (total_x, salida))

    # ---- LO QUE NINGUN FIRMWARE DE ESTE ARBOL PUEDE APROBAR -------------------
    #
    # reportar() NO CUENTA (CLAUDE.md 4). Va aqui porque es el residual honesto de este
    # cambio y no un pendiente disimulado: el sello ya existe en las tres puntas y el
    # equipo ya lo contesta, pero la orden la tiene que MANDAR alguien.
    b.reportar(
        "la app todavia no manda CMD:VERSION",
        ["el firmware de las dos puntas atiende VERSION por las dos puertas, y el sello del",
         "PUENTE si llega hoy sin tocar la app: sale en el $EVENT EVT:VERSION de cada",
         "conexion. Lo que falta es la orden desde el telefono.",
         "QUIEN LO CUENTA: app_01_comandos, comprobacion 2 -'lo que el firmware ofrece, la",
         "app lo puede usar'-, y ahi sale en ROJO hasta que la app tenga por donde",
         "preguntarlo. Es firmware sin interfaz, que es justo lo que esa comprobacion",
         "existe para cazar (N-58, SOLICITAR_PASO).",
         "Y NO SE APAGA metiendo VERSION en su lista SIN_BOTON_A_PROPOSITO: esa lista es",
         "para lo que la app NO DEBE mandar -HORA_ESP32, que es una barrera-, y esto SI",
         "debe. Apagarlo ahi seria decorar un rojo (CLAUDE.md 1)."])

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    #
    # Cada uno rompe UNA propiedad y exige que el detector lo note. Sin esto, el dia que un
    # patron dejara de casar el pack compararia nada contra nada.
    dos_ok = ('#if FW_SELLADO\n enviarTramaConCrc("$ACK,CMD:VERSION,RESULT:OK,NODE:X");\n'
              '#else\n enviarTramaConCrc("$ACK,CMD:VERSION,RESULT:OK,NODE:X");\n#endif')
    bl = _bloques_sellado(dos_ok)
    b.control_negativo(
        bool(bl) and "RESULT:OK" in bl[0][1],
        "un despachador que contesta RESULT:OK TAMBIEN sin sello se detecta: es el $ACK "
        "que no depende de lo que se pudo componer (CLAUDE.md 2)")

    # El lector del .ini tiene que saber VER el corte, no solo no verlo. Sin esta mitad,
    # A-3 podria estar aprobando porque su lector no corta nunca.
    cortado = _orden_de_sellado(_valor_ini(
        "[env:x]\nbuild_flags =\n    -D A\n    !python -c \"a ; b\"\n", "build_flags"))
    b.control_negativo(
        bool(cortado) and cortado[0] == '!python -c "a',
        "una orden con un ';' PRECEDIDO DE ESPACIO llega recortada al lector de "
        "platformio, y A-3 sabe verlo: lo que se pierde es el final del comando")

    b.control_negativo(
        any(p in "be4cd5d,SUCIO" for p in PROHIBIDOS),
        "una marca de sello con una coma dentro se detecta como capaz de partir la trama")

    b.control_negativo(
        bool([f for f in ["$STATUS,NODE:MAESTRO,SERIE:%s,FW:%s"] if "FW:" in f]),
        "un $STATUS con el sello dentro se detecta: la trama del borde del cable no lo "
        "puede llevar")

    # LA COTA SE RECALCULA, NO ESTA ESCRITA. Con un hash completo de git -40 caracteres, que
    # es lo que sale si alguien quita el --short- la desigualdad de A-8 tiene que romperse
    # en la punta mas cargada... o seguir cumpliendose por margen, y entonces lo que se
    # comprueba es que el numero CAMBIA con el ancho del hash y no es una constante.
    peor_40 = max(peores.values()) - fw_max + 40 + len(marca)
    b.control_negativo(
        peor_40 > max(peores.values()),
        "la cota de A-8 CRECE si el hash crece (%d -> %d con un hash completo de 40): esta "
        "recalculada desde la orden del .ini, no escrita a mano"
        % (max(peores.values()), peor_40))

    b.control_negativo(
        re.search(r"#if\s+defined\s*\(\s*FW_HASH\s*\)\s*&&\s*defined\s*\(\s*FW_SUCIO\s*\)",
                  "#ifdef FW_HASH") is None,
        "un sello que solo exige el hash -y no la marca de suciedad- NO pasa por A-4: "
        "declararia limpio un arbol que nadie midio")

    b.control_negativo(
        re.search(r"#define\s+FW_MARCA_SUCIO\s+\"([^\"]*)\"", "#define OTRA_COSA \"+X\"")
        is None,
        "el lector de la marca no casa con una constante de otro nombre: no hay valor por "
        "defecto que tape la desaparicion de FW_MARCA_SUCIO")
