# ===== banco/packs/documentos_04_cifras_sin_vigilante.py =====
#
# LAS CIFRAS MALAS ESTABAN JUSTO DONDE NO MIRABA NADIE.
#
# documentos_01_cifras_del_acta vigila DOS ficheros: README.md y ESTADO.md
# (documentos_01_cifras_del_acta.py:238-239). Y el 31/08 se midio donde estaban de
# verdad las cifras falsas:
#
#   MANUAL_USUARIO.md:28    "despeje de 5 a 999 s, piso minimo de 5 s"
#                           -> el C++ dice DESPEJE_SEG_MIN=10, DESPEJE_SEG_MAX=90,
#                              y el valor viaja en un uint8_t: 999 NUNCA fue
#                              representable. Y esto es SFTY-4, matriz de seguridad.
#   OPTIMIZACIONES.md:58    la misma frase, en la propia matriz de reglas.
#   MANUAL_USUARIO.md:54    "SFTY-6 a 12.0 segundos" -> son 25 s desde N-71.
#   CERTIFICACION_SW.md:20  "Maestro 42.620 B (65,0%)" -> el acta media 88,3%.
#                           Quien planificase con esa cifra creeria tener ~23 KB
#                           libres cuando quedan 7.656 B.
#
# Ninguno de los tres ficheros lo parseaba ningun pack. No es que la comprobacion
# fallara: es que NO EXISTIA. CLAUDE.md seccion 3 lo dice entero -"un instrumento que no
# esta en la compuerta no mide nada, y no deja rastro de que falta: un ABORTADO al
# menos grita; un HUECO no"-. Trece meses con el 65,0% de flash publicado es
# exactamente lo que cuesta el hueco.
#
# POR QUE UN PACK Y NO UN ARREGLO.
#
# Arreglar las cifras es media hora. Lo que impide que vuelvan es esto: CLAUDE.md
# seccion 3.ter -"un pack nuevo no es un parche: es la pasada que faltaba"-. Si los defectos
# aparecen porque alguien pregunta, no hay metodo, hay suerte.
#
# LO QUE NO HACE, Y ES DELIBERADO: NO PROHIBE LA HISTORIA.
#
# Un documento que cuenta "antes eran 12 s, desde N-71 son 25" es legitimo y valioso;
# es la convencion de este repositorio -"lo viejo se tacha con su motivo, no se
# borra"-. Un instrumento que obligue a reescribir el pasado empuja a MAQUILLARLO,
# que es peor que la cifra que venia a cazar (es la leccion del apartado 4 de
# documentos_01, sobre las rutas).
#
# Asi que este pack juzga LA AFIRMACION VIVA: antes de mirar una linea le quita los
# tramos tachados ~~asi~~, que es como este repositorio marca lo retirado. Una cifra
# vieja dentro de un tachado no se juzga; fuera de el, si.
#
# Y NO SE ANCLA AL NUMERO SUELTO: ESO YA COSTO UN FALSO VERDE HOY MISMO.
#
# documentos_01 apartado 4.bis lo tiene medido y escrito: comparar `valor in
# documento` es una SUBCADENA SOBRE EL FICHERO ENTERO, y un numero de dos digitos
# casa por accidente -"38" casaba dentro del hash `50a5380`-. Anclar a la frase
# tampoco basto, porque el documento la nombra en dos sitios. Lo que si funciono fue
# exigir que la cifra viva en la MISMA LINEA que su etiqueta. Aqui se hace igual:
#
#   - el rango de despeje se juzga en las lineas que hablan de despeje Y de
#     configurarlo, no en cualquier linea con dos numeros;
#   - el umbral de SFTY-6 se juzga en las lineas que nombran SFTY-6 o el silencio,
#     NO en las que dicen "12 s" a secas;
#   - cada cifra del acta se busca en una linea que ademas lleve su etiqueta.
#
# LOS DOS "12 s" QUE NO SON EL MISMO, Y QUE UN sed MASIVO ROMPE:
#
#   SFTY6_SILENCIO_MS = 25000UL   (protocolo.h:149)  -> publicar 12 s es MAL
#   VENTANA_TRIPLE_MS = 12000     (mando.cpp:38)     -> publicar 12 s es CORRECTO
#
# Las ventanas del mando -"A.A.A dentro de 12 s"- estan bien y aparecen en once
# lineas de estos documentos. Por eso el detector se ancla a la FRASE y no al numero,
# y por eso lleva un control negativo que le mete una linea de mando sintetica y
# exige que NO la acuse: un instrumento que da falsos positivos sobre lo correcto se
# acaba desactivando, y entonces no mide nada.

import re
import unicodedata

NOMBRE = "documentos_04_cifras_sin_vigilante"
DESCRIPCION = "los manuales y la certificacion no publican limites ni cifras que el C++ y el acta desmientan"

# Los cuatro documentos que ningun pack parseaba. README.md y ESTADO.md NO estan
# aqui a proposito: los cubre documentos_01, y duplicar la comprobacion duplicaria
# tambien el sitio donde arreglarla el dia que cambie.
DOCUMENTOS = ("MANUAL_USUARIO.md", "MANUAL_HARDWARE.md",
              "CERTIFICACION_SW.md", "OPTIMIZACIONES.md")

# ---- apartado 1: el rango de despeje ---------------------------------------------
# Una linea se juzga solo si habla del despeje Y de configurarlo. Sin el segundo
# ancla saltaria sobre prosa correcta: OPTIMIZACIONES.md:574 dice "un todo-rojo de
# 15-30 s es irrelevante" hablando de la deriva del reloj, y no es un limite de
# configuracion. Medido: con las dos anclas, el censo da 2 lineas; con una sola, 3.
TEMA_DESPEJE = ("despeje", "all-red", "all red", "todo-rojo", "todo rojo",
                "rojo estatico", "despejeseg")
ANCLA_CONFIG = ("configurable", "configurar", "configuracion", "rango", "piso ",
                "minimo", "maximo", "permite fijar")

# ---- apartado 2: el umbral de silencio de SFTY-6 ---------------------------------
# La frase, nunca el numero. Ninguna de las once lineas que hablan de la ventana del
# mando contiene una sola de estas.
FRASES_SFTY6 = ("sfty-6", "sfty6_silencio", "silencio", "sin respuesta", "sin pong",
                "perdida de comunicacion", "sin comunicacion", "orfandad")

# Y su exencion, que se midio en vez de suponerse. Con solo las frases de arriba, el
# detector acusa esta linea, que es CORRECTA:
#
#   "Sin comunicacion, la ventana del mando de 12 s sigue valiendo: el mando es local"
#
# -"sin comunicacion" casa, y el 12 es de VENTANA_TRIPLE_MS-. Un instrumento que da
# falsos positivos sobre lo correcto se acaba desactivando, y entonces no mide nada.
#
# La exencion NO es un agujero, porque lleva sus dos cerrojos: (1) la linea tiene que
# hablar el vocabulario del mando, y (2) TODAS sus cifras en segundos tienen que ser
# ventanas REALES releidas de mando.cpp -no "12", sino lo que el C++ diga hoy-. Una
# linea que nombre SFTY-6 expresamente no se exime nunca: ahi el sujeto es el umbral.
VOCAB_MANDO = ("mando", "ventana_triple", "ventana_cuadruple", "a.a.a", "b.b.b",
               "a.b.a.b", "pulsos", "secuencia")
NO_EXIME = ("sfty-6", "sfty6_silencio")

# ---- apartado 3: las cifras que CERTIFICACION_SW.md copia del acta ---------------
# (nombre de la comprobacion en el acta, patron sobre su detalle, etiqueta que la
#  linea del documento tiene que llevar, como se escribe la cifra publicada)
#
# La cuarta columna existe por lo mismo que documentos_01 escribe "N packs" y no "N":
# un numero corto suelto casa por accidente. "44" es cualquier cosa; "44 rutas" es
# la cifra.
CIFRAS_ACTA = (
    ("guarda de rutas",        r"(\d+) rutas parseadas",     "guarda de rutas",  "%s rutas"),
    ("compila maestro",        r"used (\d+) bytes",          "maestro",          "%s b"),
    ("compila maestro",        r"([\d.]+)%",                 "maestro",          "%s%%"),
    ("compila esclavo",        r"used (\d+) bytes",          "esclavo",          "%s b"),
    ("compila esclavo",        r"([\d.]+)%",                 "esclavo",          "%s%%"),
    ("compila repetidor",      r"used (\d+) bytes",          "repetidor",        "%s b"),
    ("compila repetidor",      r"([\d.]+)%",                 "repetidor",        "%s%%"),
    ("simulador funcional",    r"(\d+/\d+) PASS",            "funcional",        "%s"),
    ("simulador de repetidor", r"(\d+/\d+) PASS",            "repetidor",        "%s"),
    ("banco por packs",        r"\d+/(\d+) comprobaciones",  "banco por packs",  "%s"),
    ("arnes de pantalla",      r"TOTAL\s+(\d+/\d+)",         "pantalla",         "%s"),
    ("arnes del ciclo",        r"(\d+/\d+) comprobaciones",  "ciclo",            "%s"),
    ("arnes del automatico",   r"(\d+/\d+) comprobaciones",  "automatico",       "%s"),
    ("test funcional de la app", r"(\d+/\d+) comprobaciones", "app",             "%s"),
)

_RE_LINEA = re.compile(r"^ {2}(PASS|FALLA|ABORTADO)\s+(\S.*?)\s{2,}(\S.*)$", re.M)
_RE_CITA = r"evidencia/(\d{4}-\d{2}-\d{2})_compuerta\.txt"

# Una cantidad en segundos. El (?<![\d.,]) impide morder el interior de un numero
# mas largo -"25000ms" no da "5000"-, y exigir la unidad impide que un "12" de una
# referencia de linea o de un pin se lea como un umbral.
_RE_SEG = re.compile(r"(?<![\d.,])(\d{1,4}(?:[.,]\d+)?)\s*(?:s\b|segundos?\b|seg\b)")

# Un rango publicado: "de 10 a 90 s", "10-90 s", "entre 10 y 90 segundos".
_RE_RANGO = re.compile(
    r"(?<![\d.,])(\d{1,4})\s*(?:a|y|hasta|[-\u2013\u2014])\s*(\d{1,4})\s*"
    r"(?:s\b|segundos?\b|seg\b)")

_RE_TACHADO = re.compile(r"~~.*?~~")


def _vivo(linea):
    """La linea sin sus tramos tachados.

    Lo tachado es historia declarada, y la historia NO se juzga: prohibirla empuja a
    borrarla, y una via descartada que desaparece en silencio se vuelve a proponer.
    Se quita ANTES de normalizar porque _plano() se come las tildes de Markdown y con
    ellas los propios ~~."""
    return _RE_TACHADO.sub(" ", linea)


def _normalizar(t):
    """Deja comparable la tipografia espanola contra la del acta.

    El acta escribe 88.3% y 57880; el documento 88,3 % y puede escribir 57.880. Sin
    esto la comparacion mediria la ortografia en vez de la cifra, y un pack que falla
    por un espacio se acaba desactivando -que es como se pierde un instrumento-."""
    t = t.replace("\xa0", " ").replace("\u202f", " ").replace("\u2009", " ")
    t = re.sub(r"(?<=\d)[.,](?=\d{3}(?!\d))", "", t)   # separador de millares
    t = re.sub(r"(?<=\d),(?=\d)", ".", t)              # coma decimal
    t = re.sub(r"\s+%", "%", t)
    return t


def _plano(t):
    """Sin tildes, en minusculas y sin adornos de Markdown."""
    t = unicodedata.normalize("NFD", t)
    t = "".join(c for c in t if unicodedata.category(c) != "Mn")
    return re.sub(r"[*`_\u2022]+", "", t).lower()


def _lineas_vivas(texto):
    """[(n_linea, texto en plano y normalizado, sin lo tachado)] - 1-indexado."""
    fuera = []
    for i, l in enumerate(texto.splitlines(), 1):
        fuera.append((i, _plano(_normalizar(_vivo(l)))))
    return fuera


def _juzgadas(lineas, tema, ancla=None):
    """Las lineas que hacen la afirmacion viva que toca vigilar."""
    fuera = []
    for n, p in lineas:
        if not any(t in p for t in tema):
            continue
        if ancla is not None and not any(a in p for a in ancla):
            continue
        fuera.append((n, p))
    return fuera


def _exenta(p, norm, ventanas):
    """True si la linea habla del mando y no del umbral de SFTY-6.

    Los dos cerrojos, juntos: vocabulario del mando, TODAS sus cifras son ventanas
    reales del C++, y no nombra SFTY-6 -si lo nombra, el sujeto es el umbral y no hay
    exencion que valga-."""
    if any(x in p for x in NO_EXIME):
        return False
    return bool(norm) and any(v in p for v in VOCAB_MANDO) and norm <= ventanas


def _tipo_techo(decl):
    """Mayor valor que cabe en el tipo con el que se declara la constante.

    Sin tabla por defecto: si el tipo no esta aqui, quien llama ABORTA. Un techo
    inventado convertiria 'el 999 no cabia' en una afirmacion sin medida detras."""
    return {"uint8_t": 255, "int8_t": 127, "uint16_t": 65535,
            "int16_t": 32767, "unsigned char": 255, "char": 127}.get(decl)


def _resultados(texto_acta):
    return {n: (e, d) for e, n, d in _RE_LINEA.findall(texto_acta)}


def correr(b, fw):
    b.titulo("Los documentos que ningun pack parseaba, contra el C++ y contra el acta")

    docs = {}
    for d in DOCUMENTOS:
        docs[d] = _lineas_vivas(fw.texto_repo(d))   # ruta_repo() aborta si falta

    # =================================================================================
    # 1. EL RANGO DE DESPEJE SALE DEL C++, NO DE LA PROSA
    # =================================================================================
    b.titulo("1. El rango de despeje (SFTY-4) releido de modo_automatico.cpp")

    RUTA_AUT = ("Maestro", "src", "modo_automatico.cpp")
    dmin = fw.constante(RUTA_AUT, r"DESPEJE_SEG_MIN\s*=\s*(\d+)", "el piso de despeje")
    dmax = fw.constante(RUTA_AUT, r"DESPEJE_SEG_MAX\s*=\s*(\d+)", "el techo de despeje")

    # El tipo, no solo el valor. Es lo que convierte "999 estaba mal" en "999 no pudo
    # ser aceptado por NINGUNA version del firmware": un uint8_t no llega a 256.
    m = re.search(r"((?:unsigned\s+)?\w+)\s+DESPEJE_SEG_MIN\s*=", fw.texto(*RUTA_AUT))
    if not m:
        raise fw.Abortado(
            "no se pudo leer el TIPO con el que se declara DESPEJE_SEG_MIN en "
            "Maestro/src/modo_automatico.cpp. Sin el, la comprobacion del techo "
            "representable mediria un numero inventado")
    techo = _tipo_techo(m.group(1))
    if techo is None:
        raise fw.Abortado(
            "DESPEJE_SEG_MIN se declara como %r y este pack no sabe cual es el mayor "
            "valor que cabe en ese tipo. Poner un techo a ojo seria un valor por "
            "defecto, y un banco que no puede fallar no demuestra nada" % m.group(1))

    b.verificar(
        dmin < dmax <= techo,
        "el rango del C++ es %d-%d s y cabe en el tipo declarado (%s, techo %d)"
        % (dmin, dmax, m.group(1), techo),
        "el C++ declara DESPEJE_SEG_MIN=%d, DESPEJE_SEG_MAX=%d sobre un %s (techo "
        "%d): el propio firmware no puede aceptar su maximo"
        % (dmin, dmax, m.group(1), techo))

    publicado_en = []
    for doc, lineas in docs.items():
        for n, p in _juzgadas(lineas, TEMA_DESPEJE, ANCLA_CONFIG):
            rangos = _RE_RANGO.findall(p)
            for lo, hi in rangos:
                ok = (int(lo), int(hi)) == (dmin, dmax)
                if ok:
                    publicado_en.append("%s:%d" % (doc, n))
                b.verificar(
                    ok,
                    "%s:%d publica el rango de despeje %s-%s s, el del C++"
                    % (doc, n, lo, hi),
                    "%s:%d publica un despeje configurable de %s a %s s y el firmware "
                    "solo acepta %d-%d (modo_automatico.cpp:34). Un operario que "
                    "configure fuera de rango recibe un rechazo silencioso y se queda "
                    "con el valor anterior; y el despeje es el tiempo que garantiza "
                    "que el tramo quedo vacio -SFTY-4, matriz de seguridad-"
                    % (doc, n, lo, hi, dmin, dmax))

            # Y ningun limite publicado puede exceder lo que cabe en el tipo. Esta es
            # la comprobacion que convierte "999" en imposible y no solo en erroneo.
            for s in _RE_SEG.findall(p):
                v = float(s.replace(",", "."))
                b.verificar(
                    v <= techo,
                    "%s:%d no publica ningun limite de despeje por encima de lo "
                    "representable (%s s <= %d)" % (doc, n, s, techo),
                    "%s:%d publica %s s como tiempo de despeje configurable y el "
                    "valor viaja en un %s: no cabe de %d en adelante. No es una "
                    "configuracion desafortunada, es un numero que NINGUN firmware "
                    "posible pudo aceptar" % (doc, n, s, m.group(1), techo + 1))

    b.verificar(
        bool(publicado_en),
        "el rango %d-%d s esta publicado donde se puede leer (%s)"
        % (dmin, dmax, ", ".join(publicado_en)),
        "ninguno de los cuatro documentos publica el rango de despeje %d-%d s. Si "
        "nadie lo publica esta comprobacion no mide nada y sale verde: es el hueco "
        "que este pack vino a cerrar, reaparecido" % (dmin, dmax))

    # =================================================================================
    # 2. EL UMBRAL DE SFTY-6, Y LOS DOS "12 s" QUE NO SON EL MISMO
    # =================================================================================
    b.titulo("2. El umbral de silencio de SFTY-6 releido de protocolo.h")

    umbrales = {}
    for punta in ("Maestro", "Esclavo"):
        umbrales[punta] = fw.constante(
            (punta, "include", "protocolo.h"),
            r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
            "el umbral de silencio de SFTY-6 del %s" % punta)

    b.verificar(
        umbrales["Maestro"] == umbrales["Esclavo"],
        "las dos puntas declaran el mismo umbral de silencio (%d ms)"
        % umbrales["Maestro"],
        "el Maestro declara SFTY6_SILENCIO_MS=%d y el Esclavo %d. Con umbrales "
        "distintos una punta se va a ambar antes que la otra y ningun documento "
        "puede publicar una cifra correcta" % (umbrales["Maestro"], umbrales["Esclavo"]))

    seg6 = umbrales["Maestro"] / 1000.0
    formas = {"%g" % seg6, "%.1f" % seg6}     # "25" y "25.0"

    # Las ventanas del mando, releidas del C++ igual que el umbral. Son el OTRO "12 s"
    # -el que esta bien-, y sin leerlas del fuente la exencion seria un numero escrito
    # a mano que seguiria eximiendo el dia que la ventana cambiara.
    ventanas = {"%g" % (fw.constante(("Maestro", "src", "mando.cpp"),
                                     r"%s\s*=\s*(\d+)" % cte,
                                     "la ventana %s del mando" % cte) / 1000.0)
                for cte in ("VENTANA_TRIPLE_MS", "VENTANA_CUADRUPLE_MS")}

    vistas = []
    for doc, lineas in docs.items():
        for n, p in _juzgadas(lineas, FRASES_SFTY6):
            hallados = _RE_SEG.findall(p)
            if not hallados:
                continue        # habla de SFTY-6 sin publicar cifra: no hay nada que comparar
            norm = {"%g" % float(s.replace(",", ".")) for s in hallados}

            # La exencion del mando, con sus dos cerrojos puestos.
            if _exenta(p, norm, ventanas):
                b.reportar(
                    "%s:%d habla del mando, no del silencio de SFTY-6" % (doc, n),
                    ["sus cifras (%s s) son ventanas reales de mando.cpp (%s s)"
                     % ("/".join(sorted(norm)), "/".join(sorted(ventanas))),
                     "no se juzga contra el umbral de SFTY-6: acusarla seria un falso "
                     "positivo sobre una frase correcta"])
                continue
            ok = bool(norm & {"%g" % float(f) for f in formas})
            if ok:
                vistas.append("%s:%d" % (doc, n))
            b.verificar(
                ok,
                "%s:%d nombra el silencio de SFTY-6 y publica su umbral real (%g s)"
                % (doc, n, seg6),
                "%s:%d afirma algo sobre el silencio de SFTY-6 con cifras en segundos "
                "%s y ninguna es el umbral real, %g s (SFTY6_SILENCIO_MS=%d en las dos "
                "puntas). El techo viejo de 12 s quedaba POR DEBAJO de los ~20,8 s que "
                "el ciclo necesita para agotar sus cinco reintentos: publicarlo "
                "describe un equipo que ya no existe"
                % (doc, n, "/".join(sorted(norm)), seg6, umbrales["Maestro"]))

    b.verificar(
        bool(vistas),
        "el umbral de SFTY-6 (%g s) esta publicado donde se puede leer (%s)"
        % (seg6, ", ".join(vistas)),
        "ninguno de los cuatro documentos publica el umbral de SFTY-6. Sin una sola "
        "afirmacion viva que comparar, esta comprobacion aprueba el vacio")

    # =================================================================================
    # 3. LO QUE CERTIFICACION_SW.md PUBLICA ES LO QUE MIDIO EL ACTA
    # =================================================================================
    b.titulo("3. CERTIFICACION_SW.md contra el acta mas reciente")

    ultima = fw.actas()[0]
    res = _resultados(fw.acta(ultima))
    if not res:
        raise fw.Abortado(
            "el acta %s no se dejo partir en nombre y detalle por este pack: fallo el "
            "buscador, y comparar contra un acta vacia aprobaria el documento entero"
            % ultima)

    cert = docs["CERTIFICACION_SW.md"]

    # La cita se busca en el texto CRUDO, no en el aplanado: _plano() se come los
    # guiones bajos de Markdown y con ellos el "_compuerta.txt" del nombre del acta.
    # El pack ya se vio dar FALLA aqui sobre un documento que SI citaba bien -era el
    # buscador, no el documento (CLAUDE.md seccion 4)-.
    citadas = sorted(set(re.findall(_RE_CITA, fw.texto_repo("CERTIFICACION_SW.md"))))
    b.verificar(
        bool(citadas) and max(citadas) == ultima[:10],
        "CERTIFICACION_SW.md cita el acta mas reciente (%s)" % ultima[:10],
        "CERTIFICACION_SW.md cita %s y la ultima acta es %s. Un documento que se "
        "FIRMA y apunta a un acta vieja manda a verificar la corrida equivocada"
        % (", ".join(citadas) or "ninguna", ultima[:10]))

# SEGUNDA VUELTA DE N-112 (02/09): EL PREDICADO ERA "SALIO PASS" Y TIENE QUE SER
# "TRAE LA CIFRA".
#
# Cambiar reportar() por verificar() quito la oscilacion, pero dejo vivo un bucle: estas
# comprobaciones exigian par[0] == "PASS" en la fila del acta, y esa fila estaba en FALLA
# PRECISAMENTE PORQUE ELLAS FALLABAN. Un pack que se pide a si mismo estar en verde para
# poder ponerse en verde no se puede aprobar con ningun documento, y eso es lo que
# CLAUDE.md llama una comprobacion que ningun firmware puede aprobar: no es una
# comprobacion, es una nota.
#
# Y era ademas el predicado equivocado por el fondo: desde que compuerta.py da prioridad a
# la linea de RESUMEN, el acta trae las cifras del banco AUNQUE EL BANCO ESTE EN ROJO. Una
# cuenta en rojo es una cuenta verdadera - dice cuantas comprobaciones hay y cuantas
# cumplen-, y compararla contra el documento es exactamente lo que se quiere. Lo que
# invalida la comparacion no es que el banco falle: es que la cifra NO SE PUEDA LEER.
#
# N-112 (01/09): AQUI NO SE PUEDE USAR reportar() EN LUGAR DE verificar().
#
# Este pack tenia el mismo defecto que documentos_01 en TRES sitios, y era peor por dos
# motivos. Primero, no colgaba de si la cifra se dejaba leer sino de par[0] != "PASS",
# o sea DIRECTAMENTE del veredicto del acta. Segundo, mientras el otro pack se ponia en
# FALLA, este se quedaba en PASS midiendo dos comprobaciones menos: un verde que mide
# menos, que es la peor forma del defecto porque nadie lo mira.
#
# Y el comentario que habia aqui AFIRMABA la propiedad que el codigo incumplia -"para
# que el total de este pack no dependa de la salud del acta"- seguido de un continue que
# saltaba el verificar(). Un comentario no falla cuando alguien cambia el codigo: se
# queda describiendo un programa que ya no existe, con la autoridad de una cuenta hecha.
#
# La regla: el NUMERO de comprobaciones que emite un pack no puede depender de su propio
# veredicto ni del de nadie. Sin dato se emite en FALLA diciendo que falta, que es una
# afirmacion verdadera y util.
    for clave, patron, etiqueta, forma in CIFRAS_ACTA:
        par = res.get(clave)
        mm = re.search(patron, par[1]) if par else None
        if mm is None:
            b.verificar(
                False,
                "el acta trae la cifra de %r para poder compararla" % clave,
                "el acta %s no trae cifra legible de %r (fila: %s): sin ella no hay nada "
                "que comparar contra el documento"
                % (ultima, clave, (par[1][:60] if par else "AUSENTE")))
            continue
        if False:
            raise fw.Abortado(
                "%r salio PASS en el acta y aun asi no se pudo leer su cifra con el "
                "patron %r sobre %r: fallo el buscador, no el acta"
                % (clave, patron, par[1]))
        valor = _plano(_normalizar(forma % mm.group(1)))

        # LA CIFRA Y SU ETIQUETA, EN LA MISMA LINEA. Es la unica forma que funciono en
        # documentos_01 (apartado 4.bis): buscarla suelta en el documento entero la da
        # por buena por accidente, y anclarla solo a la frase tampoco basta cuando el
        # documento la nombra en dos sitios.
        b.verificar(
            any(etiqueta in p and valor in p for _, p in cert),
            "CERTIFICACION_SW.md publica %r junto a %r, como el acta"
            % (valor, etiqueta),
            "CERTIFICACION_SW.md NO publica %r en ninguna linea que hable de %r, y el "
            "acta %s lo midio (%r). Es un documento que alguien FIRMA: una cifra que "
            "no sale de la ultima corrida se lee como medida y no lo es"
            % (valor, etiqueta, ultima, clave))

    # EL RECUENTO DE PACKS, que el documento publica en la misma fila que el 445/445.
    # Va aparte porque no se lee de un grupo: hay que SUMAR las tres cuentas del acta
    # -"packs: 39 PASS, 0 FALLA, 0 ABORTADO"-, y se compara el TOTAL, no los que
    # pasaron, por lo mismo que documentos_01: si se comparasen los PASS, un acta roja
    # dejaria el documento imposible de cuadrar y este pack seria un cepo.
    #
    # Y va con la palabra PEGADA -"39 packs", no "39"-: es la cura del apartado 4.bis
    # de documentos_01, donde un "38" suelto casaba dentro de un hash.
    par = res.get("banco por packs")
    cuentas = re.findall(r"(\d+)\s+(?:PASS|FALLA|ABORTADO)", par[1]) if par else []
    if not cuentas:
        b.verificar(
            False,
            "el acta trae el recuento de packs para poder compararlo",
            "el acta %s no trae un recuento de packs legible (fila: %s): sin el no hay "
            "nada que comparar contra el documento"
            % (ultima, (par[1][:60] if par else "AUSENTE")))
    else:
        if not cuentas:
            raise fw.Abortado(
                "la linea de 'banco por packs' del acta %s salio PASS y aun asi no se "
                "pudo sumar su recuento de packs sobre %r: fallo el buscador" % (ultima, par[1]))
        n_packs = "%d packs" % sum(int(c) for c in cuentas)
        b.verificar(
            any("banco" in p and n_packs in p for _, p in cert),
            "CERTIFICACION_SW.md publica %r junto al banco, como el acta" % n_packs,
            "CERTIFICACION_SW.md no publica %r en ninguna linea que hable del banco, y "
            "el acta %s lo midio. Un recuento de packs viejo anuncia una cobertura que "
            "ya no es la que hay" % (n_packs, ultima))

    # Y el porcentaje de flash, en la direccion contraria: que la cifra buena aparezca
    # no basta si al lado sobrevive la mala. Este es el defecto real que se corrigio
    # hoy -"Maestro 65,0%" contra el 88,3% del acta-, y su coste no es cosmetico:
    # con el 65% se planifica estructura contando ~23 KB libres que no existen.
    for clave, etiqueta in (("compila maestro", "maestro"),
                            ("compila esclavo", "esclavo"),
                            ("compila repetidor", "repetidor")):
        par = res.get(clave)
        mflash = re.search(r"([\d.]+)%", par[1]) if par else None
        if mflash is None:
            b.verificar(
                False,
                "el acta trae el flash de %s para poder compararlo" % etiqueta,
                "el acta %s no trae un porcentaje legible de flash de %s (fila: %s): sin "
                "el no se puede buscar un segundo porcentaje en el documento"
                % (ultima, etiqueta, (par[1][:60] if par else "AUSENTE")))
            continue
        real = mflash.group(1)
        otros = sorted({x for _, p in cert if etiqueta in p and "flash" in p
                        for x in re.findall(r"([\d.]+)%", p)} - {real})
        b.verificar(
            not otros,
            "CERTIFICACION_SW.md no arrastra ningun otro porcentaje de flash del %s"
            % etiqueta,
            "CERTIFICACION_SW.md dice ADEMAS %s%% de flash del %s cuando el acta %s "
            "mide %s%%. Un porcentaje viejo vivo al lado del bueno se lee como medida"
            % ("/".join(otros), etiqueta, ultima, real))

    # =================================================================================
    # 4. CONTROLES NEGATIVOS: LAS TRES COMPROBACIONES SABEN FALLAR
    # =================================================================================
    b.titulo("4. Controles negativos")

    # (a) El defecto real, reinyectado: la frase que estuvo publicada 13 meses.
    malo = _plano(_normalizar(
        "- **Configuracion:** el menu permite configurar tiempos de despeje de 5 a "
        "999 segundos (piso minimo de 5s por seguridad vial)."))
    b.control_negativo(
        bool(_juzgadas([(1, malo)], TEMA_DESPEJE, ANCLA_CONFIG))
        and _RE_RANGO.findall(malo)
        and (int(_RE_RANGO.findall(malo)[0][0]),
             int(_RE_RANGO.findall(malo)[0][1])) != (dmin, dmax),
        "la frase '5 a 999 segundos' que este manual publico hasta el 31/08 se "
        "detecta como rango que el C++ desmiente")

    # (b) Y NO acusa a lo que esta bien. Las ventanas del mando son 12 s de verdad
    # -VENTANA_TRIPLE_MS = 12000, mando.cpp:38-, aparecen en once lineas de estos
    # documentos, y un detector anclado al numero las acusaria todas. Un instrumento
    # con falsos positivos sobre lo correcto se acaba desactivando.
    mando = _plano(_normalizar(
        "> `PB9`/`PB13` son los dos canales del mando: tres pulsos dentro de la "
        "ventana de **12 s** (`mando.cpp:38`) componen una secuencia -`A.A.A` = Modo "
        "Automatico, `B.B.B` = Ambar-, asi que el trafico cambiaria el modo solo."))
    b.control_negativo(
        not _juzgadas([(1, mando)], FRASES_SFTY6)
        and not _juzgadas([(1, mando)], TEMA_DESPEJE, ANCLA_CONFIG),
        "una linea correcta sobre la ventana de 12 s del mando NO se acusa: el "
        "detector distingue los dos '12 s' porque se ancla a la frase")

    # (b2) El caso duro, que la primera version de este pack SI acusaba -medido-: la
    # linea del mando que ademas dice "sin comunicacion". Aqui ya no basta la frase, y
    # es la exencion la que tiene que salvarla.
    duro = _plano(_normalizar(
        "Sin comunicacion, la ventana del mando de 12 s sigue valiendo: es local."))
    n_duro = {"%g" % float(s.replace(",", ".")) for s in _RE_SEG.findall(duro)}
    b.control_negativo(
        bool(_juzgadas([(1, duro)], FRASES_SFTY6)) and _exenta(duro, n_duro, ventanas),
        "una linea del mando que ademas dice 'sin comunicacion' queda exenta por sus "
        "cifras -son ventanas reales de mando.cpp-, no por la frase")

    # (b3) Y la exencion NO es un agujero: en cuanto la linea nombra SFTY-6, el sujeto
    # es el umbral y se juzga aunque hable de secuencias del mando.
    colado = _plano(_normalizar(
        "El mando compone su secuencia en 12 s, y SFTY-6 cae a ambar en 12 s."))
    n_colado = {"%g" % float(s.replace(",", ".")) for s in _RE_SEG.findall(colado)}
    b.control_negativo(
        not _exenta(colado, n_colado, ventanas)
        and not (n_colado & {"%g" % float(f) for f in formas}),
        "una linea que se escuda en el mando para publicar un umbral de SFTY-6 falso "
        "NO se exime: nombrar SFTY-6 cierra la exencion")

    # (c) La linea de SFTY-6 con el umbral viejo si se acusa.
    viejo = _plano(_normalizar(
        "1. **Perdida de Comunicacion (SFTY-6):** si se pierde comunicacion por mas "
        "de 12.0 segundos, el sistema entra en `C_FALLO`."))
    hallados = _RE_SEG.findall(viejo)
    b.control_negativo(
        bool(_juzgadas([(1, viejo)], FRASES_SFTY6)) and bool(hallados)
        and not ({"%g" % float(s.replace(",", ".")) for s in hallados}
                 & {"%g" % float(f) for f in formas}),
        "una linea que atribuye a SFTY-6 el umbral viejo de 12.0 s se detecta como "
        "cifra que protocolo.h desmiente")

    # (d) Lo tachado no se juzga: sin esto el pack obligaria a BORRAR la historia, y
    # una via descartada que desaparece en silencio se vuelve a proponer.
    historia = _plano(_normalizar(
        "*Estuvo publicado como ~~5 a 999 s~~ hasta el 31/08: era el valor anterior.*"))
    b.control_negativo(
        not _RE_RANGO.findall(_plano(_normalizar(_vivo(
            "*Estuvo publicado como ~~5 a 999 s~~ hasta el 31/08.*"))))
        and bool(_RE_RANGO.findall(historia)),
        "una cifra vieja DENTRO de un tachado deja de juzgarse, y la misma cifra sin "
        "tachar si se ve: el pack vigila la afirmacion viva, no la historia")

    # (e) El extractor de cifras del acta no cae a un valor por defecto.
    b.control_negativo(
        _resultados("\n".join(l for l in fw.acta(ultima).splitlines()
                              if "compila maestro" not in l)).get("compila maestro")
        is None,
        "si el acta no trae una comprobacion, el extractor devuelve nada en vez de un "
        "valor por defecto que daria PASS")

    # (f) Y la busqueda por MISMA LINEA distingue lo que la busqueda suelta no: el
    # fallo medido de documentos_01, reproducido aqui antes de fiarse de la cura.
    falso = [(1, "| compilacion maestro | 42620 b de flash - 65.0% | pio run |"),
             (2, "| banco por packs | 57880 b sueltos en otra fila | |")]
    b.control_negativo(
        any("57880 b" in p for _, p in falso)
        and not any("maestro" in p and "57880 b" in p for _, p in falso),
        "una cifra correcta que vive en OTRA linea del documento no se acepta como "
        "publicada: es el falso verde que costo tres intentos en documentos_01")
