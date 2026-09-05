# ===== banco/packs/app_06_formato_de_hora.py =====
#
# LA HORA QUE SALE AL CABLE SE COMPONE A MANO, NUNCA CON EL LOCALE NI CON UTC.
#
# LA PROPIEDAD, EN UNA LINEA: la hora que la app mete en SET_RTC se compone con
# getHours()/getMinutes()/getSeconds() -o toTimeString(), cuyo formato lo fija la
# especificacion- y NUNCA con toLocaleTimeString(); y la fecha se compone con
# getFullYear()/getMonth()/getDate() y NUNCA con toISOString().
#
# POR QUE ESTE PACK Y NO UNA REVISION: NO SE VE LEYENDO, Y NO APARECE POR LA MANANA.
#
# El 31/08 las dos puertas de SET_RTC formaban la hora asi:
#
#     const now = new Date().toLocaleTimeString();
#
# Es una linea que parece correcta y lo es en media Europa. MEDIDO en el equipo de
# este proyecto, cuyo locale es el de campo:
#
#     es-CO  ->  "6:25:00 p. m."      <- para las 18:25
#     es-ES  ->  "18:25:00"
#     en-US  ->  "6:25:00 PM"
#
# Y MEDIDO compilando en C el mismo sscanf que usa el firmware -Maestro/src/
# bluetooth.cpp y Esclavo/src/bluetooth.cpp, el formato se relee mas abajo-:
#
#     sscanf("2026-08-31,6:25:00 p. m.", "%d-%d-%d,%d:%d:%d")  ->  n=6, h=6
#
# El sufijo se queda fuera de la conversion, n vale 6, EL COMANDO SE ACEPTA y el
# equipo se queda con las 06:25. NADA EN LA CADENA PUEDE DETECTARLO: 06:25 es una hora
# perfectamente valida, asi que los tres $ACK son HONESTOS -miraron, y lo que miraron
# entro bien-. Y falla SOLO POR LA TARDE: cualquier prueba de manana lo aprueba.
#
# LO QUE CUELGA DE ESA HORA no es la comodidad de leerla en pantalla: es la
# autorizacion del Modo Degradado, el unico modo que enciende verde sin confirmacion
# del otro extremo.
#
# LA SEGUNDA MITAD, EN LA MISMA LINEA: la fecha salia de toISOString().slice(0, 10), y
# toISOString() es UTC. MEDIDO: local 2026-08-31 19:30 en UTC-5 -> "2026-09-01...", o
# sea que DESDE LAS 19:00 LOCALES, TODAS LAS NOCHES, el dia del mes que entra al RTC es
# el siguiente.
#
# COMO SE MIDE, SIN LEER EL CODIGO A OJO.
#
# No se busca `toLocaleTimeString` por todo el fichero -la app la usa a proposito para
# fechar las lineas del registro de eventos, que es texto de pantalla y no viaja a
# ningun sitio-. Se sigue la CADENA: de la llamada a enviarComandoFirmware('SET_RTC',
# ...) se sacan las dos expresiones de la plantilla, y cada una se resuelve hasta su
# origen -la constante que la define, la funcion que la construye, el metodo de
# js/courier_rtc.js que la devuelve-. La prohibicion se aplica sobre ESA cadena. Si
# alguna expresion no se puede resolver, esto ABORTA: aprobar una cadena que no se ha
# sabido seguir seria aprobar por no haber mirado.
#
# Y CUAL DE LAS DOS ES LA FECHA Y CUAL LA HORA NO SE SUPONE: se lee del formato del
# sscanf del C++, que es quien decide el orden.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna. La hora mal puesta socava
# SFTY-18 -la barrera que decide si la hora vale para autorizar el Degradado- pero
# este pack no la EJERCE: no comprueba reloj_enHora() ni el ano marcador, comprueba
# como compone la app la cadena que le manda. Figurar en la tabla de trazabilidad sin
# ejercer la regla es peor que una fila vacia, porque la vacia no miente.

import re

NOMBRE = "app_06_formato_de_hora"
DESCRIPCION = "la hora de SET_RTC se compone con getters locales, no con el locale ni con UTC"

PUNTAS = ("Maestro", "Esclavo")

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
COURIER_JS = ("05_Funcional", "App_Semaforo", "js", "courier_rtc.js")

# El comando cuya cadena se vigila. Se nombra aqui -no se adivina- porque es el unico
# de la app que manda un instante al equipo.
COMANDO = "SET_RTC"

# LA HORA CON SUFIJO ES EVIDENCIA, NO UNA CONSTANTE DEL FIRMWARE.
#
# Este literal no se lee del C++ a proposito, y conviene decir por que: no es un
# parametro del equipo, es la SALIDA MEDIDA de una API del navegador bajo el locale de
# campo. Se registro el 31/08 corriendo en node, en este mismo repositorio:
#
#     new Date(2026, 7, 31, 18, 25, 0).toLocaleTimeString('es-CO')  ->  "6:25:00 p. m."
#
# Sirve para una sola cosa: ejercer el sscanf y demostrar que este pack sabe ver la
# diferencia. La propiedad que de verdad se exige es estructural -componer con los
# getters-, y por eso no depende de este texto: con los getters, el locale deja de
# importar sea cual sea.
HORA_LOCALE_MEDIDA = "6:25:00 p. m."
HORA_24_EQUIVALENTE = "18:25:00"
FECHA_DE_PRUEBA = "2026-08-31"

# Lo que NO puede aparecer en la cadena que compone cada campo, y el motivo de cada uno.
PROHIBIDO = {
    "hora": {
        "toLocaleTimeString": "depende del idioma del telefono: en es-CO devuelve "
                              "'6:25:00 p. m.' para las 18:25",
        "toLocaleString": "misma familia, mismo problema, y ademas mete la fecha dentro",
    },
    "fecha": {
        "toISOString": "es UTC: desde las 19:00 locales en UTC-5 devuelve el dia "
                       "SIGUIENTE",
        "toUTCString": "UTC otra vez, con el mismo desplazamiento de dia",
        "toLocaleDateString": "depende del idioma: el orden de dia y mes cambia",
    },
}

# Lo que TIENE que aparecer. Se admiten dos formas para la hora porque las dos son
# insensibles al locale y las dos estan en uso en este arbol.
EXIGIDO_HORA = (("getHours", "getMinutes", "getSeconds"), ("toTimeString",))
EXIGIDO_FECHA = (("getFullYear", "getMonth", "getDate"),)


def _sin_comentarios(js):
    """El JavaScript con los comentarios fuera, por el mismo motivo que fuente.codigo().

    Aqui el dano va en la direccion contraria a la del C++, y por eso costo verlo: no
    es que un patron acierte dentro de un comentario y de por presente una guarda que
    no se compila, es que ACUSA. js/courier_rtc.js explica en su cabecera el defecto
    que arregla, y para explicarlo NOMBRA `toLocaleTimeString`. La primera version de
    este pack leia el fichero entero y marcaba en rojo la cadena del Courier por una
    palabra que estaba dentro de un comentario que dice justamente que ya no se usa.

    Se sustituye por un espacio -no se borra- para no pegar identificadores que estaban
    separados por un comentario."""
    js = re.sub(r"/\*.*?\*/", " ", js, flags=re.S)
    js = re.sub(r"(?m)//[^\n]*", " ", js)
    return js


def _bloque(texto, i):
    apertura = texto[i]
    cierre = {"{": "}", "(": ")", "[": "]"}[apertura]
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == apertura:
            prof += 1
        elif texto[j] == cierre:
            prof -= 1
            if prof == 0:
                return texto[i + 1:j]
    return None


def _sscanf_enteros(cadena, formato):
    """Reproduce la parte de sscanf que ESTE comando usa: %d y literales.

    No es un sscanf completo y no pretende serlo -no hay %s, ni anchos, ni *-: es
    exactamente el subconjunto que aparece en el formato que el firmware usa para
    SET_RTC, y ese formato se relee del C++ en cada corrida. Devuelve la lista de
    enteros convertidos, cuya longitud es el `n` que sscanf devolveria.

    La regla que importa y que es la del defecto: cuando el formato se agota, sscanf
    NO MIRA lo que queda de la cadena. Por eso ' p. m.' no estorba."""
    i, f, fuera = 0, 0, []
    while f < len(formato):
        if formato[f] == "%" and f + 1 < len(formato) and formato[f + 1] == "d":
            while i < len(cadena) and cadena[i] in " \t\n":
                i += 1
            j = i
            if j < len(cadena) and cadena[j] in "+-":
                j += 1
            k = j
            while k < len(cadena) and cadena[k].isdigit():
                k += 1
            if k == j:
                return fuera        # conversion fallida: sscanf se planta aqui
            fuera.append(int(cadena[i:k]))
            i, f = k, f + 2
        else:
            if i < len(cadena) and cadena[i] == formato[f]:
                i, f = i + 1, f + 1
            else:
                return fuera        # el literal no casa: sscanf se planta aqui
    return fuera


def _formato_del_cpp(fw):
    """El formato del sscanf de SET_RTC, leido de QUIEN LO PARSEA HOY: el puente.

    🔴 D-15 (05/09) - ESTE LECTOR SE MUDO, Y LA MUDANZA ES LA MITAD DEL ARREGLO.

    Hasta hoy leia la rama SET_RTC de los DOS bluetooth.cpp del STM32 y exigia que las
    dos declararan el mismo formato. Esa comprobacion tenia sentido mientras las dos
    puntas PARSEABAN la cadena: la app manda una sola y con formatos distintos habria
    entrado bien en una y mal en la otra.

    Con D-15 el reloj es el DS3231 del ESP32 y el STM32 ya no atiende SET_RTC: los dos
    sscanf desaparecieron. Dejar el lector donde estaba habria dejado este pack en
    ABORTADO -que es una puerta abierta, no una casilla pendiente (CLAUDE.md 3.quater)-,
    y aflojarlo a "si no hay sscanf, pasa" lo habria dejado en VERDE midiendo nada.

    Se muda al unico parser que queda. Y la comprobacion de "las dos puntas dicen lo
    mismo" NO se muda con el: seria vacuamente cierta -hay un solo firmware de ESP32,
    corriendo en los dos postes, asi que comparar su formato consigo mismo aprueba
    siempre-. Lo que ocupa su sitio es la comprobacion 1.bis, que exige que los STM32
    hayan dejado de parsear de verdad."""
    codigo = fw.codigo("ESP32_Expansion", "src", "despachador.cpp")
    m = re.search(r'strstr\s*\([^,]+,\s*"%s:"' % re.escape(COMANDO), codigo)
    if not m:
        return None
    m2 = re.search(r'sscanf\s*\([^,]+,\s*"([^"]+)"', codigo[m.end():])
    return m2.group(1) if m2 else None


def _rama_set_rtc_del_stm32(fw, punta):
    """El bloque de la rama SET_RTC de una punta del STM32, o None si ya no existe."""
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    m = re.search(r'strn?cmp\s*\(\s*accion\s*,\s*"%s:"' % re.escape(COMANDO), codigo)
    if not m:
        return None
    i = codigo.find("{", m.end())
    if i < 0:
        return None
    return _bloque(codigo, i)


def _envios(js):
    """[(plantilla, posicion)] de cada enviarComandoFirmware('SET_RTC', `...`)."""
    pat = re.compile(r"enviarComandoFirmware\(\s*'%s'\s*,\s*`([^`]*)`" % re.escape(COMANDO))
    return [(m.group(1), m.start()) for m in pat.finditer(js)]


def _bloque_que_contiene(js, pos):
    """El texto del bloque de llaves mas interno que contiene `pos`.

    EXISTE PORQUE SIN EL ESTE PACK NO SABIA FALLAR. Al inyectarle el defecto de vuelta
    -devolver el `const now = ahora.toLocaleTimeString()` a la puerta del reloj- el
    pack siguio en 20/20: resolvia el identificador `now` por su PRIMERA definicion en
    todo el fichero, que es la del asistente Courier, y esa seguia siendo correcta.
    Juzgaba una cadena sana mientras la de al lado estaba envenenada.

    Es la regla del instrumento: el buscador encontraba algo, y no era lo que se le
    habia pedido. Ahora cada expresion se resuelve PRIMERO en el bloque desde el que
    se manda, y solo si ahi no esta se busca en el fichero -que es como resuelve el
    propio JavaScript-."""
    pila = []
    for j, c in enumerate(js):
        if j >= pos and pila:
            ini = pila[-1]
            prof = 0
            for k in range(ini, len(js)):
                if js[k] == "{":
                    prof += 1
                elif js[k] == "}":
                    prof -= 1
                    if prof == 0:
                        return js[ini:k]
            return js[ini:]
        if c == "{":
            pila.append(j)
        elif c == "}" and pila:
            pila.pop()
    return None


def _partes_de_plantilla(plantilla):
    """Las expresiones ${...} de una plantilla, partida por las comas literales.

    Devuelve una lista de listas: un grupo de expresiones por cada campo separado por
    coma en el texto plano de la plantilla. Es lo que permite emparejar cada campo con
    su sitio en el formato del sscanf sin suponer cual va primero."""
    campos, actual, i = [], [], 0
    while i < len(plantilla):
        if plantilla.startswith("${", i):
            j = plantilla.index("}", i)
            actual.append(plantilla[i + 2:j].strip())
            i = j + 1
        elif plantilla[i] == ",":
            campos.append(actual)
            actual = []
            i += 1
        else:
            i += 1
    campos.append(actual)
    return campos


def _cuerpo_de_funcion_js(fuentes, nombre):
    """El cuerpo de una funcion JS por nombre, en cualquiera de los fuentes dados.

    LA LISTA DE PARAMETROS SE SALTA CONTANDO PARENTESIS, no con [^)]*. La primera
    version usaba `\\([^)]*\\)` y no encontraba `function fechaLocalISO(d = new Date())
    {`: el parentesis del valor por defecto cerraba antes de tiempo, la funcion se daba
    por ausente y la cadena se quedaba sin resolver. Lo cazo el pack acusando de
    `toLocaleTimeString` a un fichero que ya no la usaba.

    Cubre las tres formas que hay en este arbol: `function f(...)`, `const f = (...) =>`
    -con cuerpo o de una sola expresion- y el metodo de objeto `f(...) {` de
    js/courier_rtc.js."""
    for texto in fuentes:
        for pat in (r"\bfunction\s+%s\s*\(",
                    r"\bconst\s+%s\s*=\s*\(",
                    r"(?m)^\s*%s\s*\("):
            for m in re.finditer(pat % re.escape(nombre), texto):
                cierre = _fin_de_parentesis(texto, texto.index("(", m.end() - 1))
                if cierre is None:
                    continue
                resto = texto[cierre + 1:]
                cabeza = re.match(r"\s*(=>)?\s*", resto)
                j = cierre + 1 + cabeza.end()
                if j < len(texto) and texto[j] == "{":
                    c = _bloque(texto, j)
                    if c is not None:
                        return c
                elif cabeza.group(1):
                    # Arrow de una sola expresion: `const f = (...) => expr;`
                    fin = resto.find(";", cabeza.end())
                    return resto[cabeza.end():fin if fin > 0 else None]
    return None


def _fin_de_parentesis(texto, i):
    """El indice del ')' que cierra el '(' de texto[i]. None si no cierra."""
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "(":
            prof += 1
        elif texto[j] == ")":
            prof -= 1
            if prof == 0:
                return j
    return None


def _asignacion(fuentes, nombre):
    """La expresion con la que se define un identificador: `const X = <expr>;`."""
    for texto in fuentes:
        m = re.search(r"\b(?:const|let|var)?\s*%s\s*=\s*([^;\n]+)" % re.escape(nombre), texto)
        if m:
            return m.group(1).strip()
    return None


def _resolver(expr, fuentes, visto=None, prof=0):
    """Todo el texto del que depende `expr`, siguiendo la cadena hasta el origen.

    Devuelve None si en algun punto no sabe seguir. Ese None es deliberado y acaba en
    un ABORTADO: una cadena que el pack no ha sabido recorrer no se puede aprobar."""
    if visto is None:
        visto = set()
    if prof > 6:
        return None

    base = expr.split(".")[0].strip()
    resto = expr.split(".")[1:]

    if base in visto:
        return ""
    visto.add(base)

    definicion = _asignacion(fuentes, base)
    if definicion is None:
        return None

    acumulado = definicion

    # Si la definicion es una llamada, el origen esta dentro de la funcion llamada.
    for llamada in re.findall(r"([A-Za-z_$][\w$]*)\s*\(", definicion):
        cuerpo = _cuerpo_de_funcion_js(fuentes, llamada)
        if cuerpo is None:
            continue
        acumulado += "\n" + cuerpo
        # Y si lo que se pedia era una PROPIEDAD del resultado -comp.horaCompensada-,
        # el origen es la constante de ese nombre dentro de ese cuerpo.
        for prop in resto:
            sub = _asignacion([cuerpo], prop)
            if sub is not None:
                acumulado += "\n" + sub
                for l2 in re.findall(r"([A-Za-z_$][\w$]*)\s*\(", sub):
                    c2 = _cuerpo_de_funcion_js(fuentes, l2)
                    if c2 is not None:
                        acumulado += "\n" + c2
            elif not re.search(r"\b%s\b" % re.escape(prop), cuerpo):
                return None

    return acumulado


def correr(b, fw):
    b.titulo("El formato de la hora que la app manda en SET_RTC")

    js = _sin_comentarios(fw.texto_repo(*APP_JS))
    courier = _sin_comentarios(fw.texto_repo(*COURIER_JS))
    fuentes = (js, courier)

    # ---- 1. El formato lo dice el C++ DEL QUE PARSEA, que desde D-15 es el puente ----
    formato = _formato_del_cpp(fw)
    if formato is None:
        raise fw.Abortado(
            "no se hallo el sscanf de %s en ESP32_Expansion/src/despachador.cpp. Ese "
            "formato es quien decide que orden llevan la fecha y la hora y que se "
            "acepta, y desde D-15 el puente es el UNICO que lo parsea; sin el, este "
            "pack estaria comprobando la app contra una suposicion" % COMANDO)

    # ---- 1.bis. Y LOS DOS STM32 HAN DEJADO DE PARSEARLO DE VERDAD ----
    #
    # Ocupa el sitio de la comprobacion que antes exigia que las dos puntas declararan
    # el MISMO formato. Aquella ya no tiene sujeto -solo queda un parser- y mantenerla
    # comparando el ESP32 consigo mismo seria aprobar siempre.
    #
    # Esta si sabe fallar, y vigila la direccion que importa: que nadie devuelva el
    # segundo parser. Dos aparatos parseando la misma orden es como se llego a los DOS
    # ACUSES OPUESTOS que D-15 vino a cerrar -el puente decia OK porque la puso en su
    # DS3231 y el STM32 decia NO_QUEDO_PUESTA porque en el suyo no-, las dos ciertas.
    for punta in PUNTAS:
        rama = _rama_set_rtc_del_stm32(fw, punta)
        if rama is None:
            raise fw.Abortado(
                "el %s ya no tiene rama SET_RTC ninguna en bluetooth.cpp. Se espera que "
                "SIGA existiendo para CONSUMIR la orden en silencio: sin ella la linea "
                "cae al else del despachador y sale $ERR,CMD:DESCONOCIDO, que es otra "
                "vez una segunda respuesta a una sola orden" % punta)
        b.verificar(
            "sscanf" not in rama,
            "el %s reconoce SET_RTC pero YA NO LO PARSEA: cero sscanf en su rama" % punta,
            "el %s ha vuelto a parsear SET_RTC. D-15 dice que el reloj es del DS3231 del "
            "ESP32 y que solo el contesta; un segundo parser aqui devuelve los dos acuses "
            "opuestos a una sola orden" % punta)
        b.verificar(
            "CMD:SET_RTC" not in rama,
            "el %s no emite ningun $ACK ni $ERR con CMD:SET_RTC: una orden, un acuse" % punta,
            "el %s ha vuelto a contestar a CMD:SET_RTC. El acuse es del que tiene el "
            "reloj (D-15); dos aparatos contestando a una orden es el defecto que se "
            "cerro el 05/09" % punta)

    campos_formato = formato.split(",")
    b.verificar(
        len(campos_formato) == 2 and campos_formato[0].count("%d") == 3
        and campos_formato[1].count("%d") == 3,
        "el formato del C++ pide dos campos separados por coma, tres enteros cada uno: "
        "fecha y hora (%r)" % formato,
        "el formato del C++ ya no es 'fecha,hora' con tres enteros por campo, sino %r. "
        "Este pack empareja cada expresion de la app con su campo por ese orden, asi "
        "que hay que revisarlo antes de fiarse de sus veredictos" % formato)

    # ---- 2. El censo de puertas de SET_RTC en la app ----
    envios = _envios(js)
    if not envios:
        raise fw.Abortado(
            "app.js no tiene ni una llamada enviarComandoFirmware('%s', `...`). O la "
            "app dejo de poder poner la hora, o cambio la forma de mandarla y este "
            "buscador se quedo atras; en los dos casos medir cero puertas saldria en "
            "verde" % COMANDO)

    b.verificar(
        len(envios) >= 2,
        "censadas %d puertas de %s en app.js -el reloj del celular y la inyeccion del "
        "Courier-, leidas del fuente y no de una lista" % (len(envios), COMANDO),
        "solo se censo %d puerta de %s y la app tiene dos -el boton de sincronizar y "
        "el asistente Courier-. Una que el censo no ve es una que puede volver al "
        "locale sin que nadie lo note" % (len(envios), COMANDO))

    # ---- 3. Cada campo, resuelto hasta su origen y juzgado ahi ----
    for n, (plantilla, pos) in enumerate(envios, 1):
        # El bloque desde el que se manda va PRIMERO en la lista de fuentes: un
        # identificador se resuelve en su ambito antes que en el fichero entero.
        ambito = _bloque_que_contiene(js, pos)
        if ambito is None:
            raise fw.Abortado(
                "no se pudo delimitar el bloque de la puerta %d de %s. Sin ambito, las "
                "expresiones se resolverian por la primera definicion homonima del "
                "fichero, que es como este pack dejo de saber fallar" % (n, COMANDO))
        locales = (ambito,) + fuentes

        campos = _partes_de_plantilla(plantilla)
        if len(campos) != 2:
            raise fw.Abortado(
                "la puerta %d de %s manda `%s`, que no son dos campos separados por "
                "coma como pide el formato del C++ (%r). Sin poder emparejar campo con "
                "campo, este pack no sabe cual es la hora" % (n, COMANDO, plantilla, formato))

        for etiqueta, exprs in (("fecha", campos[0]), ("hora", campos[1])):
            if not exprs:
                raise fw.Abortado(
                    "la puerta %d de %s trae el campo de %s sin ninguna expresion "
                    "${...}: `%s`" % (n, COMANDO, etiqueta, plantilla))

            cadena = ""
            for e in exprs:
                trozo = _resolver(e, locales)
                if trozo is None:
                    raise fw.Abortado(
                        "no se pudo seguir la cadena de `%s` (campo de %s de la puerta "
                        "%d de %s) hasta su origen. Aprobar una cadena que no se ha "
                        "sabido recorrer seria aprobar por no haber mirado"
                        % (e, etiqueta, n, COMANDO))
                cadena += "\n" + trozo

            malos = [(k, v) for k, v in PROHIBIDO[etiqueta].items() if k in cadena]
            b.verificar(
                not malos,
                "puerta %d, campo de %s (`%s`): no pasa por ninguna API que dependa "
                "del idioma ni de UTC" % (n, etiqueta, ", ".join(exprs)),
                "puerta %d, campo de %s (`%s`): la cadena pasa por %s. %s"
                % (n, etiqueta, ", ".join(exprs),
                   ", ".join(k + "()" for k, _ in malos),
                   " ".join(v for _, v in malos)))

            exigido = EXIGIDO_HORA if etiqueta == "hora" else EXIGIDO_FECHA
            cumple = any(all(g in cadena for g in grupo) for grupo in exigido)
            b.verificar(
                cumple,
                "puerta %d, campo de %s: se compone con los getters locales (%s)"
                % (n, etiqueta,
                   " / ".join("+".join(g) for g in exigido if all(x in cadena for x in g))),
                "puerta %d, campo de %s (`%s`): no se compone con ninguna de las formas "
                "que no dependen del entorno (%s). Si no se compone a mano, se esta "
                "confiando en el formato que elija el telefono"
                % (n, etiqueta, ", ".join(exprs),
                   " o ".join("+".join(g) for g in exigido)))

            if etiqueta == "hora":
                # UN `|| 0` DETRAS DE UN PARSEO ES DONDE UN DATO ROTO SE VUELVE UN DATO
                # LIMPIO Y FALSO. js/courier_rtc.js hacia
                #
                #     const [hh, mm, ss] = snapshot.horaStr.split(':').map(Number);
                #     dateObj.setHours(hh || 0, mm || 0, ss || 0, 0);
                #
                # y sobre "6:25:00 p. m." el tercer trozo es "00 p. m.", cuyo Number()
                # es NaN. NaN es falsy, asi que el `|| 0` lo convertia en 0 y la
                # funcion NO REVENTABA: seguia y devolvia "06:25:05". Compensaba los
                # segundos de traslado con exactitud de cronometro sobre una hora doce
                # horas equivocada. Reventar habria sido lo barato.
                b.verificar(
                    "|| 0" not in cadena,
                    "puerta %d: la cadena de la hora no tapa un parseo fallido con un "
                    "`|| 0`" % n,
                    "puerta %d: la cadena de la hora lleva un `|| 0`. Si el parseo "
                    "falla, NaN es falsy y ese `|| 0` lo convierte en cero: no hay "
                    "excepcion, no hay aviso, y sale una hora limpia y falsa. Un "
                    "instante que no se ha podido leer se rechaza, no se rellena" % n)
                b.verificar(
                    "padStart(2" in cadena or "toTimeString" in cadena,
                    "puerta %d: la hora sale a ancho fijo -dos cifras por campo-, que "
                    "es como la documenta el manual y como se lee sin ambiguedad en un "
                    "registro" % n,
                    "puerta %d: la hora no se rellena a dos cifras. El sscanf del C++ "
                    "la acepta igual -%%d no exige ancho-, asi que esto no rompe el "
                    "ajuste; lo que rompe es la lectura: '6:25:00' en un log al lado de "
                    "'18:25:00' invita justo a la confusion que costo este defecto" % n)

    # ---- 4. El sscanf del firmware, ejercido con las dos cadenas ----
    #
    # Es la parte que demuestra POR QUE lo de arriba importa: la cadena mala no da
    # error, da una hora valida y equivocada.
    mala = "%s,%s" % (FECHA_DE_PRUEBA, HORA_LOCALE_MEDIDA)
    buena = "%s,%s" % (FECHA_DE_PRUEBA, HORA_24_EQUIVALENTE)
    n_mala = _sscanf_enteros(mala, formato)
    n_buena = _sscanf_enteros(buena, formato)

    b.verificar(
        len(n_buena) == 6 and n_buena[3] == 18,
        "con la hora compuesta a mano, el sscanf del C++ lee n=6 y hora=%d: entra la "
        "hora que el operario ve en el telefono" % (n_buena[3] if len(n_buena) > 3 else -1),
        "con la hora de 24 h el sscanf del C++ no devuelve las 18: lee %r. Si esto "
        "falla, el que esta mal es este pack -o el formato del firmware cambio-, no la "
        "app" % (n_buena,))

    b.verificar(
        len(n_mala) == 6 and n_mala[3] == 6,
        "y con la hora del locale lee n=6 y hora=6: SEIS conversiones correctas y el "
        "sufijo ' p. m.' descartado sin mirar -por eso el comando se ACEPTA y por eso "
        "ningun $ACK puede avisar-",
        "el sscanf reproducido no reproduce el defecto sobre %r: devuelve %r. La "
        "reproduccion es lo unico que sostiene el resto del pack, asi que si esto falla "
        "se arregla el pack antes de creerle nada" % (mala, n_mala))

    b.reportar(
        "el fallo no lo puede detectar ninguna de las dos puntas, y esa es su forma",
        ["Las dos cadenas producen n=6, que es la unica pregunta que el firmware hace",
         "antes de aceptar. 06:25 es una hora valida: no hay rango que la rechace, no",
         "hay checksum que la delate y el $ACK,RESULT:OK es HONESTO -miro, y lo que",
         "miro entro bien-. La unica defensa esta del lado del que compone la cadena,",
         "que es esta app, y por eso la propiedad se vigila aqui y no en el C++.",
         "Ademas falla SOLO POR LA TARDE: una prueba de manana lo aprueba entero."])

    # ---- 5. Controles negativos ----
    #
    # El mandado: reproducir el sscanf sobre la hora con sufijo y exigir 18. Da 6, o
    # sea que el detector distingue la cadena buena de la mala; si diera 18 en las dos,
    # todos los OK de arriba serian decoracion.
    b.control_negativo(
        len(n_mala) == 6 and n_mala[3] != 18 and n_buena[3] == 18,
        "el sscanf reproducido distingue las dos cadenas: '%s' entra como hora %d y "
        "'%s' como hora %d" % (HORA_LOCALE_MEDIDA, n_mala[3],
                               HORA_24_EQUIVALENTE, n_buena[3]))

    # Que la prohibicion sepa acusar: el mismo juicio sobre la cadena de ANTES.
    cadena_vieja = "const now = new Date().toLocaleTimeString();"
    b.control_negativo(
        any(k in cadena_vieja for k in PROHIBIDO["hora"]),
        "la cadena que tenia la app el 31/08 -toLocaleTimeString()- se marca como "
        "dependiente del idioma")
    cadena_vieja_fecha = "const today = new Date().toISOString().slice(0, 10);"
    b.control_negativo(
        any(k in cadena_vieja_fecha for k in PROHIBIDO["fecha"]),
        "y la fecha que tenia -toISOString()- se marca como UTC")

    # Y que NO acuse a lo que esta bien: el registro de eventos de la app usa
    # toLocaleTimeString a proposito para fechar sus lineas en pantalla. Si la
    # prohibicion fuera un grep por todo el fichero, esto saldria en rojo y el pack
    # obligaria a empeorar la app para callarlo.
    b.control_negativo(
        "toLocaleTimeString" in js and all(
            "toLocaleTimeString" not in
            (_resolver(e, (_bloque_que_contiene(js, pos),) + fuentes) or "")
            for plantilla, pos in envios for e in _partes_de_plantilla(plantilla)[1]),
        "app.js sigue usando toLocaleTimeString para el texto de pantalla y este pack "
        "NO lo acusa: mide la cadena que llega al cable, no el fichero entero")

    # Y que el seguidor de cadenas no apruebe por no haber encontrado nada.
    b.control_negativo(
        _resolver("noExisteEnNingunSitio", fuentes) is None,
        "una expresion que no se puede resolver devuelve None -y arriba eso es un "
        "ABORTADO-, en vez de una cadena vacia que pasaria todas las prohibiciones")
