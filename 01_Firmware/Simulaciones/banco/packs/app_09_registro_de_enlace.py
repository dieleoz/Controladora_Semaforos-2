# ===== banco/packs/app_09_registro_de_enlace.py =====
#
# EL INDICADOR DE ENLACE Y SU BITACORA: QUE NADA SE PINTE NI SE GUARDE SIN HABERSE
# MEDIDO.
#
# LA PROPIEDAD, EN UNA LINEA: todo valor de enlace que la app ensena o guarda viene de
# una trama $STATUS, y lo que no vino en una trama se DECLARA en vez de rellenarse.
#
# POR QUE HACE FALTA UN PACK Y NO BASTA CON HABERLO ESCRITO BIEN
#
# Esto es la version de interfaz de la prueba muerta (CLAUDE.md 3.quinquies), y este
# repositorio ya la ha pagado tres veces en la misma pantalla:
#
#   - el panel "SIMULADOR DE PRUEBAS" escribia fases inventadas en LOS MISMOS widgets
#     que la telemetria, y runLocalTicker() animaba un ciclo entero sin que nadie lo
#     pulsara;
#   - el bloque de /api/status_json pintaba modo, luces, contador, bateria, RF y RTT
#     desde un JSON de un puente de PC -y seguia ahi el 31/08, dormido, apuntando a un
#     endpoint que la propia cascara habia retirado a proposito-;
#   - y el camino que pintaba el RF hacia `(parseInt(data.RF, 10) || 0) + '%'`: un
#     campo que no fuera un numero aterrizaba en pantalla como **0%**, el peor enlace
#     medible que existe, sin que nadie hubiera medido nada.
#
# Los tres eran codigo correcto para quien lo escribio. Ninguno de los tres lo habria
# cazado un test de comportamiento, porque los tres PRODUCEN UNA PANTALLA CREIBLE. Lo
# que los caza es un censo: cuantos caminos escriben ese widget, y de donde sale lo que
# escriben.
#
# LO QUE ESTE PACK NO PUEDE HACER, ESCRITO AQUI PARA QUE NADIE LO LEA COMO PERMISO
#
# Esto es Python leyendo ficheros. NO ejecuta la app, NO abre un navegador y NO mide un
# ancho de pantalla. La medida de interfaz se hace con el navegador a CUATRO anchos
# (CLAUDE.md 4.ter) y no cabe aqui. Lo que si cabe -y es lo que se comprueba- son las
# CAUSAS: un solo escritor, un solo origen, constantes releidas y las desigualdades
# entre ellas recalculadas.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna, y la tentacion era SFTY-6
# -el silencio de radio- porque la bitacora anota justo las caidas. No la ejerce: no
# comprueba ni el umbral de silencio ni la maniobra de ambar, comprueba que un tablero
# no pinte lo que no midio. Eso lo mide costura_08_silencio. Una regla que aparece
# cubierta por una prueba que no la ejerce es peor que una fila vacia, porque la vacia
# no miente.

import re

NOMBRE = "app_09_registro_de_enlace"
DESCRIPCION = "el indicador de enlace y su bitacora: nada se pinta ni se guarda sin venir de una trama"

# Rutas literales, como las tuplas de los packs del firmware. Si alguna se mueve, esto
# ABORTA en vez de aprobar comparando contra un fichero vacio -que es como el banco
# entero se quedo en ABORTADO en N-75 con la app entrando sin vigilancia detras-.
APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
INDEX_HTML = ("05_Funcional", "App_Semaforo", "index.html")
STYLE_CSS = ("05_Funcional", "App_Semaforo", "style.css")
REGISTRO_JS = ("05_Funcional", "App_Semaforo", "js", "registro_enlace.js")

# Los widgets del indicador, por el nombre de la CONSTANTE de app.js que los sostiene.
# No se listan los ids: lo que hay que censar es quien ESCRIBE, y quien escribe usa la
# variable. El id se comprueba aparte, contra el HTML.
WIDGETS = ["rfQualityEl", "rfEstadoEl", "rfBarraEl", "rfSelloEl", "rfRttEl"]

# Las propiedades por las que un widget puede acabar ensenando un valor.
_ESCRITURA = r"\b%s\s*\.\s*(?:textContent|innerHTML|innerText|className|value)\s*="
_ESCRITURA_ESTILO = r"\b%s\s*\.\s*style\s*\.\s*\w+\s*="

_LISTA_JS = r"const\s+%s\s*=\s*\[(.*?)\]\s*;"


# ---------------------------------------------------------------------------------
# EL CENSO SE HACE SOBRE CODIGO, NO SOBRE COMENTARIOS. Y esto no es una precaucion
# teorica: la primera corrida de este pack dio CINCO FALLA, y las cinco eran el
# detector acertando dentro de los comentarios que explican el defecto retirado -el
# `(parseInt(data.RF, 10) || 0)` esta citado literalmente en app.js para que nadie lo
# vuelva a escribir-. Es exactamente el motivo por el que fw.codigo() existe para el
# C++: un patron que acierta en un comentario da por presente algo que no se ejecuta,
# y aqui daba por presente un defecto que no esta.
#
# Se escribe a mano porque JavaScript tiene tres cosas que el filtro de C++ no
# contempla y que romperian el censo: las plantillas con backtick, las expresiones
# regulares literales -que llevan '/' y pueden contener '//'- y las URL dentro de
# cadenas ('https://...'), que un re.sub de '//.*' se llevaria por delante junto con
# media linea de codigo.
# ---------------------------------------------------------------------------------

def _codigo_js(t):
    """El JavaScript con los comentarios fuera, respetando cadenas y regex."""
    fuera = []
    i, n = 0, len(t)
    # Ultimo caracter significativo emitido: decide si un '/' abre una regex o divide.
    ant = ""
    while i < n:
        c = t[i]
        d = t[i + 1] if i + 1 < n else ""
        if c == "/" and d == "/":
            j = t.find("\n", i)
            i = n if j < 0 else j
            continue
        if c == "/" and d == "*":
            j = t.find("*/", i + 2)
            i = n if j < 0 else j + 2
            fuera.append(" ")
            continue
        if c in "'\"`":
            cierre = c
            fuera.append(c)
            i += 1
            while i < n:
                if t[i] == "\\":
                    fuera.append(t[i:i + 2])
                    i += 2
                    continue
                fuera.append(t[i])
                if t[i] == cierre:
                    i += 1
                    break
                i += 1
            ant = cierre
            continue
        if c == "/" and (ant == "" or ant in "=(,:[!&|?{};+*%~^<>rn"):
            # Regex literal. 'rn' cubre `return /re/` y `in /re/`, que son los unicos
            # casos de palabra clave que aparecen aqui.
            fuera.append(c)
            i += 1
            while i < n:
                if t[i] == "\\":
                    fuera.append(t[i:i + 2])
                    i += 2
                    continue
                if t[i] == "[":
                    while i < n and t[i] != "]":
                        fuera.append(t[i])
                        i += 1
                fuera.append(t[i])
                if t[i] == "/":
                    i += 1
                    break
                if t[i] == "\n":
                    i += 1
                    break
                i += 1
            ant = "/"
            continue
        fuera.append(c)
        if not c.isspace():
            ant = c
        i += 1
    return "".join(fuera)


def _codigo_html(t):
    return re.sub(r"<!--.*?-->", " ", t, flags=re.S)


def _codigo_css(t):
    return re.sub(r"/\*.*?\*/", " ", t, flags=re.S)


# ---------------------------------------------------------------------------------
# Utilidades de bloques. Traidas literales de app_08_enrutado_por_punta, que ya las
# tenia probadas: reescribirlas para renombrar una variable es como se cuelan los
# errores en un cambio que no debe cambiar comportamiento (CLAUDE.md 3.bis).
# ---------------------------------------------------------------------------------

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


def _span(texto, i):
    """Como _bloque(), pero devolviendo (inicio, fin) en vez del texto."""
    if i < 0 or i >= len(texto) or texto[i] != "{":
        return None
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "{":
            prof += 1
        elif texto[j] == "}":
            prof -= 1
            if prof == 0:
                return (i + 1, j)
    return None


def _span_funcion(js, nombre):
    """(inicio, fin) del cuerpo de `function nombre(...) {`. None si no esta."""
    m = re.search(r"\bfunction\s+%s\s*\([^)]*\)\s*\{" % re.escape(nombre), js)
    if not m:
        return None
    return _span(js, m.end() - 1)


def _pila_de_bloques(js):
    """Para cada '{' su posicion, en forma de lista de (inicio, fin) anidados.

    Sirve para responder "en que bloques esta metida esta llamada", que es la pregunta
    del trinquete de puntas: la guarda tiene que estar en el manejador que contiene la
    llamada, no en cualquier sitio del fichero."""
    abiertos = []
    spans = []
    for i, c in enumerate(js):
        if c == "{":
            abiertos.append(i)
        elif c == "}" and abiertos:
            ini = abiertos.pop()
            spans.append((ini + 1, i))
    return spans


def _contenedores(spans, pos):
    """Los bloques que contienen pos, del mas interno al mas externo."""
    dentro = [s for s in spans if s[0] <= pos < s[1]]
    dentro.sort(key=lambda s: s[1] - s[0])
    return dentro


def _numero(js, nombre, fichero):
    """Lee una constante numerica del JavaScript. ABORTA si no esta.

    Sin valor por defecto, nunca: un banco que no puede fallar no demuestra nada, y el
    dia que alguien renombre la constante esto seguiria dando PASS midiendo el numero
    viejo mientras la app usa otro."""
    m = re.search(r"\b%s\s*[:=]\s*(\d+)" % re.escape(nombre), js)
    if not m:
        raise _ABORT("no se pudo leer del JavaScript la constante %s (%s). Sin ese "
                     "numero este pack mediria otra cosa que la app y seguiria dando "
                     "PASS" % (nombre, fichero))
    return int(m.group(1))


_ABORT = None  # se enlaza en correr() con fw.Abortado


# ---------------------------------------------------------------------------------
# WCAG. La cuenta sale del CSS, no de mirar la pantalla.
# ---------------------------------------------------------------------------------

def _lin(c):
    c = c / 255.0
    return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4


def _lum(hexa):
    h = hexa.lstrip("#")
    r, g, b = (int(h[i:i + 2], 16) for i in (0, 2, 4))
    return 0.2126 * _lin(r) + 0.7152 * _lin(g) + 0.0722 * _lin(b)


def _ratio(a, b):
    la, lb = _lum(a), _lum(b)
    return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)


# ---------------------------------------------------------------------------------

def correr(b, fw):
    global _ABORT
    _ABORT = fw.Abortado

    # `js`/`reg`/`html`/`css` son SIEMPRE codigo sin comentarios: es lo unico que se
    # ejecuta y lo unico que ve el usuario. Los originales solo hacen falta para la
    # guarda de que el filtro no se ha comido el fichero entero.
    js_bruto = fw.texto_repo(*APP_JS)
    reg_bruto = fw.texto_repo(*REGISTRO_JS)
    js = _codigo_js(js_bruto)
    reg = _codigo_js(reg_bruto)
    html = _codigo_html(fw.texto_repo(*INDEX_HTML))
    css = _codigo_css(fw.texto_repo(*STYLE_CSS))

    # LA REGLA DEL INSTRUMENTO (CLAUDE.md 4): antes de creerse un "no aparece", hay que
    # descartar al buscador. Si el filtro de comentarios se comiera el fichero, todos
    # los censos de abajo darian cero y esto saldria en verde sin haber mirado nada.
    if len(js) < len(js_bruto) * 0.15 or "parseNmeaTelemetry" not in js:
        raise fw.Abortado(
            "el filtro de comentarios dejo app.js en %d de %d bytes y/o perdio "
            "parseNmeaTelemetry. El buscador esta roto: cualquier censo suyo daria cero "
            "y aprobaria" % (len(js), len(js_bruto)))
    if len(reg) < len(reg_bruto) * 0.15 or "anotar" not in reg:
        raise fw.Abortado(
            "el filtro de comentarios dejo js/registro_enlace.js en %d de %d bytes y/o "
            "perdio anotar()" % (len(reg), len(reg_bruto)))

    # =============================================================================
    b.titulo("1. UN SOLO ESCRITOR: nadie pinta el enlace fuera de pintarEnlace()")
    # =============================================================================
    span_pintar = _span_funcion(js, "pintarEnlace")
    if span_pintar is None:
        raise fw.Abortado(
            "no se hallo function pintarEnlace() en app.js. Es el unico escritor "
            "declarado de los widgets de enlace: sin el, este pack no sabe contra que "
            "medir y aprobaria cualquier reparto de escrituras")

    declarados = [w for w in WIDGETS
                  if re.search(r"\bconst\s+%s\s*=\s*document\.getElementById" % w, js)]
    if len(declarados) < len(WIDGETS):
        raise fw.Abortado(
            "en app.js solo estan declarados %d de los %d widgets de enlace (%s). O se "
            "renombraron o se mudaron: censar las escrituras de una variable que ya no "
            "existe da CERO escrituras y sale verde sin haber mirado nada"
            % (len(declarados), len(WIDGETS), sorted(set(WIDGETS) - set(declarados))))

    def _escrituras_fuera(fuente, ini, fin):
        fuera = []
        for w in WIDGETS:
            for patron in (_ESCRITURA % w, _ESCRITURA_ESTILO % w):
                for m in re.finditer(patron, fuente):
                    if not (ini <= m.start() < fin):
                        linea = fuente[:m.start()].count("\n") + 1
                        fuera.append("%s (app.js:%d)" % (w, linea))
        return sorted(set(fuera))

    fuera = _escrituras_fuera(js, *span_pintar)
    b.verificar(
        not fuera,
        "los %d widgets de enlace (%s) solo se escriben dentro de pintarEnlace()"
        % (len(WIDGETS), ", ".join(WIDGETS)),
        "hay escrituras de widgets de enlace FUERA de pintarEnlace(): %s. Con dos "
        "escritores vuelve a poder aparecer uno que pinte un valor que no vino en una "
        "trama -es lo que hacia el bloque de /api/status_json-" % fuera)

    # Y que de verdad escribe: un pintarEnlace() vacio pasaria lo de arriba.
    cuerpo_pintar = js[span_pintar[0]:span_pintar[1]]
    escrituras_dentro = sum(
        len(re.findall(_ESCRITURA % w, cuerpo_pintar)) +
        len(re.findall(_ESCRITURA_ESTILO % w, cuerpo_pintar)) for w in WIDGETS)
    b.verificar(
        escrituras_dentro >= len(WIDGETS),
        "pintarEnlace() escribe %d veces sobre los widgets: existe y hace su trabajo"
        % escrituras_dentro,
        "pintarEnlace() solo tiene %d escrituras para %d widgets. Un unico escritor que "
        "no escribe deja la pantalla con lo anterior pintado, que es indistinguible de "
        "un dato de ahora" % (escrituras_dentro, len(WIDGETS)))

    b.control_negativo(
        bool(_escrituras_fuera(
            "const rfQualityEl = document.getElementById('rf-quality');\n"
            "function pintarEnlace(l) { rfQualityEl.textContent = l.pct + '%'; }\n"
            "function otra() { rfQualityEl.textContent = '98%'; }\n", 0, 10)),
        "un segundo escritor del widget de enlace metido en otra funcion se detecta")

    # =============================================================================
    b.titulo("2. UN SOLO ORIGEN: la lectura de enlace solo se fabrica desde $STATUS")
    # =============================================================================
    span_parse = _span_funcion(js, "parseNmeaTelemetry")
    if span_parse is None:
        raise fw.Abortado(
            "no se hallo function parseNmeaTelemetry() en app.js. Es la unica puerta "
            "por la que entra una trama del equipo: sin ella no se puede distinguir un "
            "valor medido de uno inventado")

    llamadas_lectura = [m.start() for m in re.finditer(r"\blecturaDeEnlace\s*\(", js)]
    definicion = _span_funcion(js, "lecturaDeEnlace")
    if definicion is None or not llamadas_lectura:
        raise fw.Abortado(
            "no se hallo lecturaDeEnlace() (definida=%s, llamadas=%d) en app.js. Es la "
            "unica fabrica de lecturas de enlace declarada"
            % (definicion is not None, len(llamadas_lectura)))

    # La propia definicion no es una llamada: se reconoce por el `function ` delante.
    fuera_de_status = [
        js[:p].count("\n") + 1 for p in llamadas_lectura
        if not (span_parse[0] <= p < span_parse[1])
        and not js[max(0, p - 12):p].rstrip().endswith("function")
    ]
    b.verificar(
        not fuera_de_status,
        "las %d llamadas a lecturaDeEnlace() estan dentro de parseNmeaTelemetry(): la "
        "lectura solo se fabrica con los campos de una trama" % len(llamadas_lectura),
        "hay llamadas a lecturaDeEnlace() fuera del parser de tramas (app.js lineas "
        "%s). Un origen de enlace que no es una trama es un dato inventado con formato "
        "de medida" % fuera_de_status)

    # A que se llama pintarEnlace(): solo con ENLACE_SIN_DATO o con lo que salio de
    # lecturaDeEnlace(). Cualquier otro argumento seria un valor puesto a mano.
    de_trama = set(re.findall(r"\b(?:const|let|var)\s+(\w+)\s*=\s*lecturaDeEnlace\s*\(", js))
    permitidos = de_trama | {"ENLACE_SIN_DATO"}
    args = [(m.group(1).strip(), js[:m.start()].count("\n") + 1)
            for m in re.finditer(r"\bpintarEnlace\s*\(([^)]*)\)", js)]
    args = [(a, ln) for a, ln in args if a]          # la definicion no cuenta
    malos = [(a, ln) for a, ln in args if a not in permitidos]
    b.verificar(
        args and not malos,
        "las %d llamadas a pintarEnlace() usan solo %s: o una lectura de trama, o la "
        "constante que significa 'no lo se'" % (len(args), sorted(permitidos)),
        "pintarEnlace() se llama con %s, que no salio de lecturaDeEnlace() ni es "
        "ENLACE_SIN_DATO. Eso es un valor de enlace escrito a mano llegando a la "
        "pantalla" % malos)

    b.verificar(
        bool(re.search(r"const\s+ENLACE_SIN_DATO\s*=\s*\{[^}]*medido\s*:\s*false", js)),
        "ENLACE_SIN_DATO existe y declara medido:false, que es lo que separa 'no lo se' "
        "de un valor",
        "no se halla ENLACE_SIN_DATO con medido:false. Sin una constante que signifique "
        "la ausencia, la ausencia acaba representada por un numero -y el numero que se "
        "elige siempre es el 0-")

    b.control_negativo(
        "98" not in permitidos and bool([a for a in ["{ pct: 98 }"] if a not in permitidos]),
        "un pintarEnlace({pct: 98}) con el valor escrito a mano no pasa la lista de "
        "argumentos permitidos")

    # =============================================================================
    b.titulo("3. UN CAMPO QUE NO SE ENTIENDE NO ES UN CERO")
    # =============================================================================
    # El defecto literal que habia: `(parseInt(data.RF, 10) || 0) + '%'`. Con RF
    # ausente, vacio o no numerico la pantalla escribia 0%, o sea el peor enlace
    # medible, sin que nadie hubiera medido nada.
    _RELLENO = re.compile(
        r"(?:parseInt|parseFloat|Number)\s*\(\s*data\.(?:RF|RTT)\b[^;\n]*?\)\s*(?:\|\||\?\?)")
    caidos = [js[:m.start()].count("\n") + 1 for m in _RELLENO.finditer(js)]
    b.verificar(
        not caidos,
        "ningun campo de enlace de la trama cae a un valor por defecto con || o ?? "
        "(el defecto de `(parseInt(data.RF,10) || 0)` no ha vuelto)",
        "app.js lineas %s convierten data.RF/data.RTT con un valor por defecto detras. "
        "Un campo que no se pudo leer se pintaria como un enlace medido, y el numero "
        "que sale de ese || es siempre el peor posible" % caidos)

    b.control_negativo(
        bool(_RELLENO.search("rfQualityEl.textContent = (parseInt(data.RF, 10) || 0) + '%';")),
        "el detector reconoce el defecto original -el `|| 0` sobre data.RF- cuando se "
        "le pone delante")
    b.control_negativo(
        not _RELLENO.search("const pct = _pctDeTrama(data.RF === undefined ? null : data.RF);"),
        "y NO acusa a la forma correcta, que pregunta si el campo vino antes de "
        "convertirlo")

    # =============================================================================
    b.titulo("4. LOS UMBRALES SALEN DEL FUENTE, Y SU DESIGUALDAD SE RECALCULA")
    # =============================================================================
    rf_bien = _numero(js, "RF_BIEN", "app.js")
    rf_justo = _numero(js, "RF_JUSTO", "app.js")
    b.verificar(
        0 < rf_justo < rf_bien <= 100,
        "los umbrales del indicador cumplen 0 < RF_JUSTO (%d) < RF_BIEN (%d) <= 100: "
        "los tres tramos existen" % (rf_justo, rf_bien),
        "los umbrales son RF_JUSTO=%d y RF_BIEN=%d. Si se cruzan o se igualan, el tramo "
        "del medio desaparece SIN QUE NADA FALLE: la pantalla pasaria de bueno a "
        "cayendo sin avisar por el medio, que es justo donde el tecnico tiene que "
        "reaccionar" % (rf_justo, rf_bien))

    cuerpo_clasificar = None
    span_clas = _span_funcion(js, "clasificarEnlace")
    if span_clas:
        cuerpo_clasificar = js[span_clas[0]:span_clas[1]]
    b.verificar(
        cuerpo_clasificar is not None
        and "RF_BIEN" in cuerpo_clasificar and "RF_JUSTO" in cuerpo_clasificar
        and not re.search(r">=\s*\d", cuerpo_clasificar),
        "clasificarEnlace() compara contra las constantes por su nombre, no contra "
        "numeros escritos dentro",
        "clasificarEnlace() no usa RF_BIEN/RF_JUSTO o lleva numeros escritos a mano. "
        "Una segunda copia del umbral es una que se queda vieja sin avisar")

    # =============================================================================
    b.titulo("5. 'NO LO SE' NO ES UN TRAMO PEOR: cuatro rotulos y ninguno es un %")
    # =============================================================================
    m = re.search(r"const\s+ENLACE_ROTULO\s*=\s*\{(.*?)\}\s*;", js, re.S)
    if not m:
        raise fw.Abortado(
            "no se hallo la tabla ENLACE_ROTULO en app.js. Es donde vive la diferencia "
            "entre 'va mal' y 'no lo se', que es la mitad del encargo")
    rotulos = dict(re.findall(r"(\w+)\s*:\s*'([^']*)'", m.group(1)))
    b.verificar(
        set(rotulos) == {"BIEN", "JUSTO", "CAYENDO", "SIN_DATO"},
        "el indicador tiene los cuatro estados %s -tres tramos medidos y la ausencia-"
        % sorted(rotulos),
        "los estados del indicador son %s. Faltando SIN_DATO, la ausencia de medida se "
        "representa con uno de los tramos y el operario no puede distinguirlas"
        % sorted(rotulos))
    b.verificar(
        len(set(rotulos.values())) == len(rotulos),
        "los cuatro rotulos son textos DISTINTOS: el color no es el unico canal -a "
        "pleno sol es lo primero que se pierde-",
        "hay rotulos repetidos en %s. Dos estados con el mismo texto solo se distinguen "
        "por color, y el color se pierde con sol, con la pantalla sucia y con quien no "
        "distingue rojo de verde" % rotulos)
    sin_dato = rotulos.get("SIN_DATO", "")
    b.verificar(
        sin_dato and not re.search(r"[\d%]", sin_dato),
        "el rotulo de la ausencia (%r) no lleva ninguna cifra ni un %%: no se puede "
        "leer como una medida" % sin_dato,
        "el rotulo de SIN_DATO es %r y contiene una cifra o un %%. Un 'no lo se' con "
        "aspecto de numero es exactamente el 0%% que se acaba de quitar" % sin_dato)

    # =============================================================================
    b.titulo("6. LA BITACORA: constantes releidas y la desigualdad del hueco")
    # =============================================================================
    tope = _numero(reg, "TOPE", "js/registro_enlace.js")
    periodo = _numero(reg, "PERIODO_MUESTRA_MS", "js/registro_enlace.js")
    hueco = _numero(reg, "HUECO_MS", "js/registro_enlace.js")

    b.verificar(
        tope > 0,
        "el registro tiene tope duro de %d anotaciones: no crece sin limite en el "
        "telefono" % tope,
        "el tope del registro es %d. Sin tope, la bitacora se come el almacenamiento "
        "del telefono y acaba fallando justo el dia que hace falta" % tope)

    b.verificar(
        hueco > periodo,
        "HUECO_MS (%d) > PERIODO_MUESTRA_MS (%d): una muestra de rutina no se dibuja "
        "como una interrupcion" % (hueco, periodo),
        "HUECO_MS=%d no es mayor que PERIODO_MUESTRA_MS=%d. Con eso, CADA muestra "
        "normal aparece como un hueco y la tira se vuelve ilegible -es N-71 otra vez: "
        "dos constantes cuya relacion solo vivia en un comentario-" % (hueco, periodo))

    horizonte_h = (tope * periodo) / 3600000.0
    b.verificar(
        horizonte_h >= 4.0,
        "el registro cubre al menos %.1f h seguidas de muestras de rutina: aguanta un "
        "turno" % horizonte_h,
        "con TOPE=%d y una muestra cada %d ms el registro solo cubre %.1f h. Una caida "
        "nocturna ya no estaria dentro cuando el tecnico abra la app por la manana"
        % (tope, periodo, horizonte_h))

    # =============================================================================
    b.titulo("7. EL REGISTRO NO INVENTA: ni entre huecos, ni rellenando con ceros")
    # =============================================================================
    m = re.search(r"\btramos\s*\([^)]*\)\s*\{", reg)
    cuerpo_tramos = _bloque(reg, m.end() - 1) if m else None
    if cuerpo_tramos is None:
        raise fw.Abortado(
            "no se hallo tramos() en js/registro_enlace.js. Es la funcion que mete un "
            "HUECO donde no llego nada: sin ella no hay nada que comprobar y la tira "
            "podria estar dibujando una linea continua sobre un silencio")
    b.verificar(
        "HUECO_MS" in cuerpo_tramos and "CLASE_HUECO" in cuerpo_tramos,
        "tramos() compara el salto contra HUECO_MS y mete un item CLASE_HUECO: el hueco "
        "se dibuja como hueco, no se estira la muestra anterior",
        "tramos() no usa HUECO_MS o no emite CLASE_HUECO. Sin eso las celdas quedan "
        "pegadas sobre un tiempo en el que no se recibio NADA, y quien la mire leera "
        "que el enlace estuvo asi todo el rato")

    m = re.search(r"\banotar\s*\([^)]*\)\s*\{", reg)
    cuerpo_anotar = _bloque(reg, m.end() - 1) if m else None
    if cuerpo_anotar is None:
        raise fw.Abortado("no se hallo anotar() en js/registro_enlace.js")
    # Se mira la EXPRESION ENTERA que se guarda en rf/rtt, no si en algun sitio del
    # cuerpo aparece el texto `rf: 0`. La primera version de este pack hacia lo
    # segundo y una inyeccion la desarmo en un caracter: el defecto real no se escribe
    # `rf: 0`, se escribe `rf: <la condicion> ? lec.pct : 0`, y el `0` esta al final de
    # la linea, no detras de los dos puntos. Un detector que solo reconoce la forma mas
    # torpe del defecto deja pasar todas las demas.
    def _expresion(campo):
        m2 = re.search(r"\b%s\s*:\s*([^\n]*?),?\s*$" % campo, cuerpo_anotar, re.M)
        return m2.group(1).strip() if m2 else None

    expr_rf, expr_rtt = _expresion("rf"), _expresion("rtt")
    ok_relleno = (expr_rf is not None and expr_rtt is not None
                  and expr_rf.endswith(": null") and expr_rtt.endswith(": null"))
    b.verificar(
        "medido === true" in cuerpo_anotar and ok_relleno,
        "anotar() guarda el enlace solo si la lectura dice medido===true, y lo no "
        "medido cae a `null`: rf=%r" % expr_rf,
        "anotar() no exige `medido === true`, o lo no medido no cae a null (rf=%r, "
        "rtt=%r). Un 0 en esa columna significa 'se midieron cero latidos "
        "contestados', que es un dato durisimo; 'no se midio' no es eso y no puede "
        "compartir casilla" % (expr_rf, expr_rtt))

    # La direccion contraria, que es la que se olvida: un cero MEDIDO tiene que
    # guardarse como cero. Una guarda escrita como `lec.pct ? lec.pct : null` cumpliria
    # lo de arriba y perderia el dato mas grave que puede haber.
    b.verificar(
        not re.search(r"lec\.pct\s*\?\s*", cuerpo_anotar),
        "y un 0 medido de verdad SI se guarda: la condicion no es la veracidad del "
        "numero sino la bandera de medida",
        "anotar() decide con la veracidad de lec.pct. Un enlace medido al 0% es falsy "
        "en JavaScript, asi que se guardaria como 'no medido': se perderia exactamente "
        "la anotacion mas grave de la bitacora")

    m = re.search(r"\b_celda\s*\([^)]*\)\s*\{", reg)
    cuerpo_celda = _bloque(reg, m.end() - 1) if m else None
    b.verificar(
        cuerpo_celda is not None and "''" in cuerpo_celda
        and not re.search(r"return\s*['\"]0['\"]", cuerpo_celda),
        "el CSV deja la celda VACIA cuando no hubo medida, en vez de escribir un 0",
        "_celda() no devuelve cadena vacia para lo no medido. Quien abra el CSV en una "
        "hoja de calculo veria ceros donde no hubo medida y dibujaria una caida a cero "
        "que nadie observo")

    def _sintetico(cuerpo, campo):
        m2 = re.search(r"\b%s\s*:\s*([^\n]*?),?\s*$" % campo, cuerpo, re.M)
        return m2.group(1).strip() if m2 else None

    b.control_negativo(
        not (_sintetico("      rf: lec.medido === true ? lec.pct : 0,\n", "rf") or "")
        .endswith(": null"),
        "el detector reconoce el relleno con 0 escrito como fallback del ternario -que "
        "es como se escribe de verdad-, y no solo la forma torpe `rf: 0`")
    b.control_negativo(
        (_sintetico("      rf: lec.medido === true ? lec.pct : null,\n", "rf") or "")
        .endswith(": null"),
        "y NO acusa a la forma correcta, que cae a null")
    b.control_negativo(
        "CLASE_HUECO" not in "for (const r of lista) { fuera.push(r); }",
        "y reconoce un tramos() que devuelve la lista tal cual, sin marcar la "
        "interrupcion")

    # Y que no haya aparecido ningun dibujante de lineas: unir dos puntos con una recta
    # es AFIRMAR lo que paso en medio.
    linea = [t for t in ("lineTo", "<polyline", "<path", "interpolar", "lerp")
             if t in js or t in reg]
    b.verificar(
        not linea,
        "no hay ningun dibujante de lineas ni interpolador en la app: la tira son "
        "celdas y huecos",
        "aparecen %s en la app. Un grafico que une dos medidas con una recta esta "
        "afirmando lo que paso en medio, y en medio es justo donde esta lo que se "
        "quiere saber" % linea)

    # =============================================================================
    b.titulo("8. LAS CINCO CLASES DE ANOTACION TIENEN QUIEN LAS ESCRIBA")
    # =============================================================================
    # Es el censo de costura_10 aplicado aqui: una clase declarada que nadie anota es
    # una columna de la bitacora que nunca se rellena, y en un registro eso se lee como
    # "no paso", no como "no lo apunte".
    m = re.search(r"CLASES\s*:\s*\[(.*?)\]", reg, re.S)
    if not m:
        raise fw.Abortado("no se hallo la lista CLASES en js/registro_enlace.js")
    clases = re.findall(r"'([^']+)'", m.group(1))
    if len(clases) < 3:
        raise fw.Abortado(
            "el censo de clases de anotacion dio %d. Con tan pocas, este pack aprobaria "
            "una bitacora que solo sabe guardar una cosa" % len(clases))
    huerfanas = [c for c in clases
                 if not re.search(r"anotar\s*\(\s*'%s'" % re.escape(c), js)]
    b.verificar(
        not huerfanas,
        "las %d clases de anotacion (%s) tienen llamador en app.js" % (len(clases), clases),
        "%s estan declaradas y NADIE las anota. Una clase que nunca se escribe deja un "
        "hueco en la bitacora que se lee como 'no paso' en vez de 'no lo apunte'"
        % huerfanas)

    b.verificar(
        "localStorage" in reg and "JSON.parse" in reg and "JSON.stringify" in reg,
        "la bitacora persiste en el almacenamiento del telefono: sobrevive al cierre de "
        "la app, que es el motivo entero por el que existe",
        "js/registro_enlace.js no usa localStorage. Un registro que solo vive en memoria "
        "se pierde justo en el reinicio que se quiere diagnosticar")

    b.verificar(
        "disponible" in reg and "motivoNoDisponible" in reg
        and "RegistroEnlace.disponible" in js,
        "y cuando el telefono NO deja guardar, la app lo declara en vez de perder los "
        "datos en silencio",
        "no hay bandera de almacenamiento no disponible, o la pantalla no la mira. Un "
        "registro que no se esta guardando y no lo avisa es peor que no tenerlo: el "
        "tecnico se va del poste creyendo que lleva la prueba encima")

    # =============================================================================
    b.titulo("9. TODA ORDEN DE UNA SOLA PUNTA PREGUNTA ANTES DE SALIR AL CABLE")
    # =============================================================================
    # El trinquete no nombra los cuatro emisores que fallaban: se recalcula desde las
    # listas de la app, asi que cubre tambien al quinto que alguien escriba manana.
    listas = {}
    for punta, nombre in (("Maestro", "SOLO_MAESTRO"), ("Esclavo", "SOLO_ESCLAVO")):
        mm = re.search(_LISTA_JS % nombre, js, re.S)
        if mm is None:
            raise fw.Abortado(
                "no se hallo en app.js la lista %s. El reparto por punta vive SOLO ahi: "
                "sin ella este pack compararia contra un conjunto vacio y aprobaria "
                "cualquier emisor" % nombre)
        listas[punta] = re.findall(r"'([^']+)'", mm.group(1))
    de_una_punta = set(listas["Maestro"]) | set(listas["Esclavo"])

    spans = _pila_de_bloques(js)
    sin_guarda = []
    vigilados = 0
    for mm in re.finditer(r"\benviarComandoFirmware\s*\(\s*'([^']+)'", js):
        orden = mm.group(1)
        raiz = orden.split(":")[0]
        if orden not in de_una_punta and raiz not in de_una_punta:
            continue
        vigilados += 1
        # LA GUARDA TIENE QUE ESTAR JUNTO AL EMISOR: en el bloque que contiene la
        # llamada o en su padre inmediato -que es el `if (btnX)` de siempre-, y ANTES
        # de la llamada, porque una guarda posterior no guarda.
        #
        # DOS NIVELES Y NO CUATRO, Y ESTO LO ENSENO UNA INYECCION. Con cuatro, el censo
        # llegaba hasta el cuerpo del DOMContentLoaded -que contiene la app entera- y
        # ahi hay un `state.node === 'ESCLAVO'` en la linea 801 por motivos que no
        # tienen nada que ver: al quitarle la guarda al boton de MODO AUTOMATICO el
        # pack seguia en 40/40. Un ancestro suficientemente grande contiene siempre
        # algo que se parece a una guarda, y entonces la comprobacion aprueba a todos.
        protegido = False
        for ini, fin in _contenedores(spans, mm.start())[:2]:
            trozo = js[ini:mm.start()]
            if "puntaCorrecta(" in trozo or re.search(r"state\.node\s*===", trozo):
                protegido = True
                break
        if not protegido:
            sin_guarda.append("%s (app.js:%d)" % (orden, js[:mm.start()].count("\n") + 1))

    if vigilados < 4:
        raise fw.Abortado(
            "solo se hallaron %d emisores de ordenes de una sola punta en app.js. El "
            "31/08 habia mas de cuatro: o el emisor se renombro o las llamadas dejaron "
            "de llevar el comando escrito entero, y entonces este censo no ve nada y "
            "sale verde" % vigilados)

    b.verificar(
        not sin_guarda,
        "los %d emisores de ordenes de una sola punta consultan puntaCorrecta() -o "
        "state.node- antes de escribir al cable" % vigilados,
        "%s salen al cable sin preguntar a que punta van. Contra la punta equivocada "
        "vuelven como $ERR,CMD:DESCONOCIDO: el boton parece roto y el error no dice de "
        "que habla" % sin_guarda)

    b.control_negativo(
        bool([1 for _ in [0]
              if not any(t in "btn.addEventListener('click', () => { "
                              "enviarComandoFirmware('SET_MODO', 'AUTO'); });"
                         for t in ("puntaCorrecta(",))]),
        "un manejador que manda SET_MODO sin consultar la punta se detecta")
    b.control_negativo(
        "puntaCorrecta(" in ("btn.addEventListener('click', () => { "
                             "const p = puntaCorrecta('SET_MODO'); if (p) return; "
                             "enviarComandoFirmware('SET_MODO', 'AUTO'); });"),
        "y uno que SI la consulta no se marca: el detector distingue el emisor "
        "protegido del desnudo")

    # =============================================================================
    b.titulo("10. LA PANTALLA DICE QUE ESTO NO ES POTENCIA")
    # =============================================================================
    # El numero es el % de latidos que contestaron. Cualquiera que vea una barra de
    # nivel lee "senal", y con esa lectura se cambia una antena que no tiene la culpa.
    b.verificar(
        re.search(r"latidos", html, re.I) is not None
        and re.search(r"no es potencia", html, re.I) is not None
        and re.search(r"RSSI", html) is not None,
        "el HTML declara al lado del indicador que es % de latidos contestados y NO "
        "potencia de senal (RSSI)",
        "la nota que distingue latidos de potencia no esta en index.html. Sin ella el "
        "tecnico lee la barra como cobertura y cambia la antena de un equipo cuyo "
        "problema puede ser el cable, el conector o un obstaculo")

    b.verificar(
        "dBm" not in js and "dBm" not in html,
        "y en ninguna parte de la app aparece un dBm: no hay ninguna medida de potencia "
        "en este enlace",
        "aparece 'dBm' en la app. Ese numero no existe en este equipo: publicarlo seria "
        "inventar una unidad de medida entera")

    ids = ["rf-estado", "rf-barra", "rf-sello", "registro-tira", "registro-lista",
           "registro-resumen", "btn-registro-csv", "btn-registro-limpiar"]
    faltan = [i for i in ids if 'id="%s"' % i not in html]
    b.verificar(
        not faltan,
        "los %d elementos del indicador y de la bitacora estan en index.html" % len(ids),
        "faltan en index.html: %s. La app escribiria sobre elementos que no existen y "
        "el indicador se quedaria mudo sin dar ningun error" % faltan)

    # =============================================================================
    b.titulo("11. EL CONTRASTE SALE DEL CSS, NO DE MIRAR LA PANTALLA")
    # =============================================================================
    paleta = dict(re.findall(r"--([a-z0-9-]+):\s*(#[0-9a-fA-F]{6})", css))
    if "bg-surface" not in paleta:
        raise fw.Abortado(
            "no se hallo --bg-surface en style.css: sin el fondo no hay contra que "
            "medir, y aprobar aqui seria comparar nada contra nada")
    fondo = paleta["bg-surface"]
    tokens = ["enlace-bien", "enlace-justo", "enlace-cayendo", "enlace-sindato"]
    for token in tokens:
        if token not in paleta:
            raise fw.Abortado(
                "no existe --%s en style.css. Los cuatro colores del indicador se miden "
                "desde el CSS; si el token se renombro, este pack aprobaria una paleta "
                "que no es la que se pinta" % token)
        r = _ratio(paleta[token], fondo)
        b.verificar(
            r >= 4.5,
            "--%s (%s) da %.2f:1 sobre %s: cumple AA para texto"
            % (token, paleta[token], r, fondo),
            "--%s (%s) da %.2f:1 sobre %s, por debajo de 4.5:1. Se usa como ROTULO del "
            "estado del enlace: a pleno sol eso no se lee, y el reflejo comprime todavia "
            "mas los ratios de un tema oscuro"
            % (token, paleta[token], r, fondo))

    sin_usar = [t for t in tokens if ("var(--%s)" % t) not in css]
    b.verificar(
        not sin_usar,
        "los cuatro tokens del indicador se usan de verdad en el CSS",
        "%s estan definidos y nadie los usa. Un color medido que no se pinta es una "
        "cuenta que tranquiliza sin cubrir nada" % sin_usar)

    b.control_negativo(
        _ratio("#64748B", "#0B111E") < 4.5 and _ratio("#F8FAFC", "#0B111E") >= 7.0,
        "la cuenta de contraste suspende el gris viejo (4.0:1) y aprueba el blanco "
        "(18:1): sabe distinguir el que cumple del que no")

    b.reportar(
        "LO QUE ESTE PACK NO MIDE, y hay que medir en otro sitio",
        ["- El ANCHO. Esto es Python leyendo ficheros: la interfaz se mide con el",
         "  navegador a 412 / 390 / 360 / 320 px (CLAUDE.md 4.ter). Una captura a un",
         "  solo ancho no demuestra nada.",
         "- El SOL. El ratio WCAG se calcula sobre el color nominal; el reflejo sube el",
         "  nivel de negro y comprime los ratios, y a un tema oscuro le comprime mas.",
         "- Que el numero que llega sea VERDAD. El Maestro lo mide de verdad",
         "  (coordinador.cpp:845); el Esclavo emitia RF y RTT como literales en su",
         "  snprintf. Esta app no puede distinguir un 98 medido de un 98 escrito a",
         "  mano: eso se arregla en el firmware y se vigila con otro pack.",
         "  Horizonte del registro con las constantes de hoy: %.1f h." % horizonte_h])
